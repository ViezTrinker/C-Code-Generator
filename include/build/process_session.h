/*!
 *\file process_session.h
 *\brief Interactive child-process session with streamed stdout and stdin.
 */
#ifndef PROCESS_SESSION_H
#define PROCESS_SESSION_H

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

#include "model/result.h"

namespace Cgen
{
   class ProcessSession;

   /*!
    *\brief Background stdout reader for ProcessSession.
    *
    *\param[in,out] pSession Session that owns the pipes.
    */
   void ProcessSessionReaderMain(ProcessSession* pSession);

   /*!
    *\brief Runs a program without blocking the UI, streaming I/O.
    */
   class ProcessSession
   {
   public:
      /*!
       *\brief Constructs an idle session.
       */
      ProcessSession(void);

      /*!
       *\brief Stops any running child on destruction.
       */
      ~ProcessSession(void);

      ProcessSession(const ProcessSession& other) = delete;
      ProcessSession& operator=(const ProcessSession& other) = delete;

      /*!
       *\brief Starts \p executablePath as a child process.
       *
       *\param[in] executablePath Absolute or relative path to the binary.
       *\return Result::Ok on success.
       */
      Result Start(std::string_view executablePath);

      /*!
       *\brief Writes text to the child's stdin.
       *
       *\param[in] text Bytes to write (typically a line including '\\n').
       *\return Result::Ok on success.
       */
      Result WriteStdin(std::string_view text);

      /*!
       *\brief Requests the child process to terminate.
       */
      void Stop(void);

      /*!
       *\brief Returns true while the child is still running.
       */
      bool IsRunning(void) const;

      /*!
       *\brief Appends newly captured stdout/stderr into \p pOutText.
       *
       *\param[out] pOutText Destination string.
       *\return true if any bytes were appended.
       */
      bool ConsumeOutput(std::string* pOutText);

      /*!
       *\brief Child exit code after it has finished, or 0 while running.
       */
      int32_t GetExitCode(void) const;

   private:
      friend void ProcessSessionReaderMain(ProcessSession* pSession);

      void CloseHandles(void);
      void JoinReader(void);
      void AppendOutput(std::string_view text);

      mutable std::mutex _mutex;
      std::string _outputQueue;
      std::thread _readerThread;
      bool _running = false;
      int32_t _exitCode = 0;
#ifdef _WIN32
      void* _pProcessHandle = nullptr;
      void* _pThreadHandle = nullptr;
      void* _pStdinWrite = nullptr;
      void* _pStdoutRead = nullptr;
#endif
   };
} // namespace Cgen

#endif // PROCESS_SESSION_H
