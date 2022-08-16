magick tiles/0008_big.png -interpolative-resize 1024 -interpolate bilinear tiles/0008.png
magick tiles/0010_big.png -interpolative-resize 1024 -interpolate bilinear tiles/0010.png

magick montage -mode concatenate -background None -tile 4x tiles/%04d.png[0-15] tile.png
magick composite -compose Over -tile overlay.png tile.png PNG24:tile.png