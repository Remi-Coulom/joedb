#!/bin/bash
set -e

# https://github.com/divi255/sphinxcontrib.asciinema

# recording tmux session with asciinema:
# https://gist.github.com/worldofprasanna/1861b182103cef452ec58471679a7e5b

# https://superuser.com/questions/265320/disable-the-status-bar-in-tmux
# tmux set -g status off

# https://stackoverflow.com/questions/29753909/tmux-borders-has-two-colors-on-mac-terminal
# tmux set-option -g pane-border-style fg=green

# https://unix.stackexchange.com/questions/32986/how-do-i-equally-balance-tmux1-split-panes
# tmux select-layout even-vertical

sudo apt install python3-venv
mkdir -p ~/.venvs
python3 -m venv ~/.venvs/sphinx
source ~/.venvs/sphinx/bin/activate
pip install sphinx_rtd_theme
pip install sphinxcontrib.spelling
pip install sphinxcontrib.doxylink
pip install sphinx-favicon
pip install sphinx-sitemap

cd ~/repos

if [ ! -d sphinxcontrib.asciinema ]; then
 git clone git@github.com:Remi-Coulom/sphinxcontrib.asciinema.git
fi

cd sphinxcontrib.asciinema
pip install .
