# Gravity Snapshot

This program is based off this [video](https://www.youtube.com/watch?v=C5Jkgvw-Z6E). But instead of simulating a pendulum swinging above magnets, we're simulating a point being attracted to masses. And since this gravity simulation can go on for a lot longer than the pendulum, or might never settle at all, we color the starting pixel based off of where the point ends up after a certain number of iterations.

There are three masses layed out to form an equilateral triangle. Each mass represents one of red, green, and blue. To color the pixel we set the color channel associated with the clossest mass to 255, the other two channels are set to proportionately to their distance from the point. This is totally arbitrary, I chose it because it creates the most interesting images from the other things I tried.

You can create then animate the effect of increasing the number of iterations, see `--help` for more options.
To turn the rendered frames into a video with ImageMagick you can do: `convert -quality 100 *.bmp video.webm`