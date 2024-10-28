Simple Scheduler Workflow

1. Initialization:
   Accepts number_of_cpus and timeslice as command-line arguments.

2. Command-Line Interface:
   Prompt: simple-shell>.
   Commands:
      "exit": Runs all submitted processes, then exits.
      "history": Displays command history.
      "submit [command]": Adds a command to the ready queue.

3. Command Execution:
   Adds each command to history.
   For "submit", parses and adds a process with optional priority to ready_queue.

4. Round-Robin Execution (on "exit"):
   Runs each process for timeslice, prioritizing by level.
   Completed processes are logged and removed; incomplete ones re-queued.

5. Process Management:
   Starts or resumes processes as needed.
   Checks completion after each timeslice, logging completed processes.

6. Output Results:
   After all processes finish, displays each process’s:
      Name, priority, total execution time, and wait time.

7. Termination:
   Frees memory and exits.


Contribution

1. Turbold Amarbat: 2023559:
   Process & History Management and Bonus Part
2. Tung Duc Vu: 2023558:
   Execution & Scheduling


Github:
https://github.com/TungDucVu/OS_Assignment3