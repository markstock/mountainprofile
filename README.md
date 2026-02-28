# mountainprofile
Generate an attractive profile from a DEM/DSM of a mountain

## Usage
On a Linux computer with basic software development packages installed, you should be able to do the following:

	make
	./mountainplots.bin -i dsm.png -o profile.png --elevs 1000 5000 --mpp 5

For example, here is a DEM of the Matterhorn at 5 meters per pixel, followed by the result of

	./mountainplots.bin -i Matterhorn5m.png --elevs 1647 4748 --mpp 5 -d 1.5 -o MatterhornProfile.png

![Digital elevation model of Matterhorn at 5 meters per pixel](Matterhorn5m.png =400x)

![Graphical profile of the Matterhorn at 5 meters per pixel](MatterhornProfile.png =400x)

## Motivation
The measure of a mountain is far more than its summit elevation.
While "prominence" is an esaily accessible second measurement, I find that
no small set of numbers can communicate the complex character of a mountain.
This program is my attempt at measuring and classifying mountains.

It generates an "x-ray" of the surface of the mountain, given by a DEM (digital
elevation model) or DSM (digital surface model) in png format (we recommend
using 16 bits per channel, greyscale only), rotated around the summit point.
The image sets to bottom pixel row at sea level and uses the spatial resolution
as the vertical resolution (5 meters per pixel in the above example).

The resulting image communicates much of the character of a mountain: how high
is it, how does it rise from its base, how conical or symmetric is it, how steep are 
its most dramatic walls, how steep is the typical slope, are there neighboring
or sub-peaks, etc.

## Credits
Thanks to the developers of [CLI11](https://github.com/CLIUtils/CLI11) and libpng.

I don't get paid for writing or maintaining this, so if you find this tool
useful or mention it in your writing, please please cite it by using the 
following BibTeX entry.

```
@Misc{mountainprof2026,
  author =       {Mark J.~Stock},
  title =        {mountainprofile: Generate an attractive profile from a DEM/DSM of a mountain},
  howpublished = {\url{https://github.com/markstock/mountainprofile}},
  year =         {2026}
}
```
