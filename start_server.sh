#!/bin/bash

SESSION_NAME="ArduinoServer"
PROJECT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Check if the tmux session already exists
if tmux has-session -t $SESSION_NAME 2>/dev/null; then
    echo "Session '$SESSION_NAME' already exists."
    tmux attach-session -t $SESSION_NAME
    exit 0
fi

echo "Creating new tmux session: $SESSION_NAME"

# Create a new detached tmux session
tmux new-session -d -s $SESSION_NAME -n "Monitoring"

# Start the server in the first pane
tmux send-keys -t $SESSION_NAME:0.0 "source venv/bin/activate" C-m
tmux send-keys -t $SESSION_NAME:0.0 "cd ArduinoServer" C-m
tmux send-keys -t $SESSION_NAME:0.0 "python run.py" C-m

# Split the window vertically
tmux split-window -v -t $SESSION_NAME:0

# Start the reader in the second pane
tmux send-keys -t $SESSION_NAME:0.1 "source venv/bin/activate" C-m
tmux send-keys -t $SESSION_NAME:0.1 "cd ArduinoServer" C-m
tmux send-keys -t $SESSION_NAME:0.1 "python -m app.reader" C-m

# Attach to the session
tmux attach-session -t $SESSION_NAME
