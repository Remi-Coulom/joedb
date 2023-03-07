#!/bin/bash
#https://github.com/OfekShilon/optview2
output_dir=optview2
~/repos/optview2/opt-viewer.py -j4 --output-dir ${output_dir} --source-dir .. .
echo xdg-open ${output_dir}/index.html
