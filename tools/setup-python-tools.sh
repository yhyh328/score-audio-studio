#!/usr/bin/env bash

set -euo pipefail

python3 -m venv .venv

.venv/bin/python -m pip install --upgrade pip
.venv/bin/python -m pip install -r tools/requirements.txt

echo "Python tools environment is ready."
echo "Activate it with:"
echo "source .venv/bin/activate"
