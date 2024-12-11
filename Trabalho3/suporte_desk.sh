#!/bin/bash

PIPE_SUPPORT="/tmp/suporte"
PIPE_ADMIN="/tmp/admin"
LOCKFILE="/tmp/suporte_desk.lock"

trap 'rm -f $PIPE_SUPPORT /tmp/student_* $PIPE_ADMIN $LOCKFILE; pkill -f support_agent; exit' INT TERM EXIT

if [ $# -ne 4 ]; then
    echo "Usage: $0 <NALUN> <NDISCIP> <NLUG> <NSTUD>"
    exit 1
fi

NALUN=$1  # Number of students
NDISCIP=$2  # Number of disciplines
NLUG=$3  # Number of slots per discipline
NSTUD=$4  # Number of student processes

# Cleanup old pipes
echo "Removing old pipes..."
rm -f "$PIPE_SUPPORT" "$PIPE_ADMIN" /tmp/student_*

# Create main communication pipe
echo "Creating main pipe ($PIPE_SUPPORT)..."
mkfifo "$PIPE_SUPPORT"
chmod 0666 "$PIPE_SUPPORT"

# Create admin pipe
echo "Creating admin pipe ($PIPE_ADMIN)..."
mkfifo "$PIPE_ADMIN"
chmod 0666 "$PIPE_ADMIN"

# Start support agent
echo "Starting support agent..."
./suporte_agente "$NALUN" &
sleep 1

# Start student processes
echo "Starting $NSTUD student processes..."
students_per_process=$((NALUN / NSTUD))
for ((i=1; i<=NSTUD; i++)); do
    start_student=$((1 + (i-1)*students_per_process))
    if [ $i -eq $NSTUD ]; then
        # Last process gets remaining students
        num_students=$((NALUN - (i-1)*students_per_process))
    else
        num_students=$students_per_process
    fi
    
    echo "Starting student process $i: start=$start_student, count=$num_students"
    ./student "$i" "$start_student" "$num_students" &
    sleep 0.1
done

# Wait for all background processes
wait

# Cleanup
echo "Cleaning up..."
rm -f "$PIPE_SUPPORT" "$PIPE_ADMIN" /tmp/student_*
rm -f "$LOCKFILE"
echo "Execution finished."