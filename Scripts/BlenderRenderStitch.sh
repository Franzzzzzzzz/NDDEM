#!/bin/bash



ffmpeg -i render%d.png -i renderD4-%d.png -i renderD5-%d.png \
-filter_complex '[0:v]transpose=3[a];[a]pad=iw*3:ih[int];[1:v]transpose=3[b];[2:v]transpose=3[c];[int][b]overlay=W/3:0[int2];[int2][c]overlay=2*W/3:0[vid];[vid]drawtext=text=D3:x=50:y=50:fontfile=OpenSans.ttf:fontsize=72:fontcolor=white[vid];[vid]drawtext=text=D4:x=w/3+50:y=50:fontfile=OpenSans.ttf:fontsize=72:fontcolor=white[vid];[vid]drawtext=text=D5:x=2*w/3+50:y=50:fontfile=OpenSans.ttf:fontsize=72:fontcolor=white[vid];[vid]drawbox=iw/3:20:5:ih-40:white[vid];[vid]drawbox=2*iw/3:20:5:ih-40:white[vid]' \
-map [vid] \
-q 1 -y RenderMultiD.mp4
