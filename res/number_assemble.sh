magick montage -mode concatenate -background None -tile 4x numbers/%04d.png[0-15] number_0.png
magick composite -compose Over -tile overlay.png number_0.png PNG24:number_0.png

magick montage -mode concatenate -background None -tile 4x numbers/%04d.png[16-31] number_1.png
magick composite -compose Over -tile overlay.png number_1.png PNG24:number_1.png