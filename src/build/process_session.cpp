/*!
 *\file process_session.cpp
 *\brief Windows interactive process session implementation.
 */
#include "build/process_session.h"

#include <array>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Cgen
{
   ProcessSession::ProcessSession(void) = default;

   ProcessSession::~ProcessSession(void)
   {
      Stop();
   }

   void ProcessSession::AppendOutput(std::string_view text)
   {
      std::lock_guard<std::mutex> lock(_mutex);
      _outputQueue.append(text);
      constexpr size_t MaxQueued = 100000;
      if (_outputQueue.size() > MaxQueued)
      {
         _outputQueue.erase(0, _outputQueue.size() - MaxQueued);
      }
   }

   void ProcessSession::CloseHandles(void)
   {
#ifdef _WIN32
      if (_pStdinWrite != nullptr)
      {
         CloseHandle(static_cast<HANDLE>(_pStdinWrite));
         _pStdinWrite = nullptr;
      }
      if (_pStdoutRead != nullptr)
      {
         CloseHandle(static_cast<HANDLE>(_pStdoutRead));
         _pStdoutRead = nullptr;
      }
      if (_pThreadHandle != nullptr)
      {
         CloseHandle(static_cast<HANDLE>(_pThreadHandle));
         _pThreadHandle = nullptr;
      }
      if (_pProcessHandle != nullptr)
      {
         CloseHandle(static_cast<HANDLE>(_pProcessHandle));
         _pProcessHandle = nullptr;
      }
#endif
   }

   void ProcessSession::JoinReader(void)
   {
      if (_readerThread.joinable())
      {
         _readerThread.join();
      }
   }

   bool ProcessSession::IsRunning(void) const
   {
      std::lock_guard<std::mutex> lock(_mutex);
      return _running;
   }

   int32_t ProcessSession::GetExitCode(void) const
   {
      std::lock_guard<std::mutex> lock(_mutex);
      return _exitCode;
   }

   bool ProcessSession::ConsumeOutput(std::string* pOutText)
   {
      if (pOutText == nullptr)
      {
         return false;
      }
      std::lock_guard<std::mutex> lock(_mutex);
      if (_outputQueue.empty())
      {
         return false;
      }
      pOutText->append(_outputQueue);
      _outputQueue.clear();
      return true;
   }

   Result ProcessSession::WriteStdin(std::string_view text)
   {
#ifdef _WIN32
      HANDLE stdinWrite = nullptr;
      {
         std::lock_guard<std::mutex> lock(_mutex);
         if ((!_running) || (_pStdinWrite == nullptr))
         {
            return Result::Error;
         }
         stdinWrite = static_cast<HANDLE>(_pStdinWrite);
      }
      DWORD written = 0;
      const BOOL ok = WriteFile(stdinWrite,
                                text.data(),
                                static_cast<DWORD>(text.size()),
                                &written,
                                nullptr);
      if ((!ok) || (written != static_cast<DWORD>(text.size())))
      {
         return Result::IoError;
      }
      return Result::Ok;
#else
      (void)text;
      return Result::Error;
#endif
   }

   void ProcessSession::Stop(void)
   {
#ifdef _WIN32
      HANDLE processHandle = nullptr;
      HANDLE stdinWrite = nullptr;
      {
         std::lock_guard<std::mutex> lock(_mutex);
         processHandle = static_cast<HANDLE>(_pProcessHandle);
         stdinWrite = static_cast<HANDLE>(_pStdinWrite);
         _pStdinWrite = nullptr;
      }
      if (stdinWrite != nullptr)
      {
         CloseHandle(stdinWrite);
      }
      if (processHandle != nullptr)
      {
         TerminateProcess(processHandle, 1);
      }
#endif
      JoinReader();
      {
         std::lock_guard<std::mutex> lock(_mutex);
         _running = false;
      }
      CloseHandles();
   }

   Result ProcessSession::Start(std::string_view executablePath)
   {
      Stop();

#ifdef _WIN32
      SECURITY_ATTRIBUTES securityAttributes {};
      securityAttributes.nLength = sizeof(securityAttributes);
      securityAttributes.bInheritHandle = TRUE;

      HANDLE stdoutRead = nullptr;
      HANDLE stdoutWrite = nullptr;
      HANDLE stdinRead = nullptr;
      HANDLE stdinWrite = nullptr;

      if (!CreatePipe(&stdoutRead, &stdoutWrite, &securityAttributes, 0))
      {
         return Result::IoError;
      }
      if (!SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0))
      {
         CloseHandle(stdoutRead);
         CloseHandle(stdoutWrite);
         return Result::IoError;
      }
      if (!CreatePipe(&stdinRead, &stdinWrite, &securityAttributes, 0))
      {
         CloseHandle(stdoutRead);
         CloseHandle(stdoutWrite);
         return Result::IoError;
      }
      if (!SetHandleInformation(stdinWrite, HANDLE_FLAG_INHERIT, 0))
      {
         CloseHandle(stdoutRead);
         CloseHandle(stdoutWrite);
         CloseHandle(stdinRead);
         CloseHandle(stdinWrite);
         return Result::IoError;
      }

      STARTUPINFOA startupInfo {};
      startupInfo.cb = sizeof(startupInfo);
      startupInfo.dwFlags = STARTF_USESTDHANDLES;
      startupInfo.hStdInput = stdinRead;
      startupInfo.hStdOutput = stdoutWrite;
      startupInfo.hStdError = stdoutWrite;

      PROCESS_INFORMATION processInfo {};
      std::string command = "\"";
      command.append(executablePath);
      command.append("\"");

      std::array<char, 1024> commandBuffer {};
      if (command.size() >= commandBuffer.size())
      {
         CloseHandle(stdoutRead);
         CloseHandle(stdoutWrite);
         CloseHandle(stdinRead);
         CloseHandle(stdinWrite);
         return Result::InvalidArgument;
      }
      for (size_t index = 0; index < command.size(); ++index)
      {
         commandBuffer[index] = command[index];
      }

      const BOOL created = CreateProcessA(nullptr,
                                          commandBuffer.data(),
                                          nullptr,
                                          nullptr,
                                          TRUE,
                                          CREATE_NO_WINDOW,
                                          nullptr,
                                          nullptr,
                                          &startupInfo,
                                          &processInfo);

      CloseHandle(stdoutWrite);
      CloseHandle(stdinRead);

      if (!created)
      {
         CloseHandle(stdoutRead);
         CloseHandle(stdinWrite);
         return Result::IoError;
      }

      {
         std::lock_guard<std::mutex> lock(_mutex);
         _pProcessHandle = processInfo.hProcess;
         _pThreadHandle = processInfo.hThread;
         _pStdinWrite = stdinWrite;
         _pStdoutRead = stdoutRead;
         _running = true;
         _exitCode = 0;
         _outputQueue.clear();
      }

      _readerThread = std::thread(ProcessSessionReaderMain, this);
      return Result::Ok;
#else
      (void)executablePath;
      return Result::Error;
#endif
   }

   void ProcessSessionReaderMain(ProcessSession* pSession)
   {
      if (pSession == nullptr)
      {
         return;
      }

#ifdef _WIN32
      HANDLE stdoutRead = nullptr;
      HANDLE processHandle = nullptr;
      {
         std::lock_guard<std::mutex> lock(pSession->_mutex);
         stdoutRead = static_cast<HANDLE>(pSession->_pStdoutRead);
         processHandle = static_cast<HANDLE>(pSession->_pProcessHandle);
      }

      std::array<char, 512> buffer {};
      while (stdoutRead != nullptr)
      {
         DWORD bytesRead = 0;
         const BOOL ok = ReadFile(stdoutRead,
                                  buffer.data(),
                                  static_cast<DWORD>(buffer.size() - 1),
                                  &bytesRead,
                                  nullptr);
         if ((!ok) || (bytesRead == 0))
         {
            break;
         }
         pSession->AppendOutput(std::string_view(buffer.data(), bytesRead));
      }

      DWORD exitCode = 0;
      if (processHandle != nullptr)
      {
         WaitForSingleObject(processHandle, INFINITE);
         GetExitCodeProcess(processHandle, &exitCode);
      }

      {
         std::lock_guard<std::mutex> lock(pSession->_mutex);
         pSession->_exitCode = static_cast<int32_t>(exitCode);
         pSession->_running = false;
      }
#else
      (void)pSession;
#endif
   }
} // namespace Cgen
