# Vector field

Given a set of sparse 2D vectors (e.g. wind vectors observed at weather stations), the program displays a smooth 2D vector field in which vectors are displayed as moving points or arrows. A standard inverse distance weighted (IDW) interpolation is used to calculate values for the vector field.

**Contact**: Toan Nguyen ([http://monash.edu/mivp](http://monash.edu/mivp))

## Omegalib module

Tested with Omegalib 13.1

Compile:
```
cd vectorfield
mkdir build
cd build
cmake ..
make
```

Run with python script:
```
import vectorfield
vf = vectorfield.initialize()
vf.init(0, 0, 109379.614343, 72919.7428954, 4000, 1000);

vf.addControlPoint( 72195.6097426 , 10213.8282815 , 4.9193495505 , 9.838699101 );
vf.addControlPoint( 80216.7814611 , 8390.83470912 , 6.36396103068 , 6.36396103068 );
vf.addControlPoint( 19693.3948579 , 52142.6804464 , 10.6066017178 , 10.6066017178 );
vf.addControlPoint( 91519.3416099 , 67820.4251689 , -6.36396103068 , 6.36396103068 );
vf.addControlPoint( 12036.8218539 , 21151.7897158 , 9.19238815543 , 9.19238815543 );
vf.addControlPoint( 9484.63085255 , 33548.146008 , 8.049844719 , 4.0249223595 );
vf.addControlPoint( 38652.5280107 , 23703.9807172 , 7.6026311235 , 15.205262247 );
vf.addControlPoint( 64539.0367386 , 39017.1267252 , 4.9193495505 , 9.838699101 );
vf.addControlPoint( 101728.105615 , 44850.7061568 , 8.4970583145 , 16.994116629 );
vf.addControlPoint( 58340.8585925 , 10943.0257105 , 14.1421356237 , 14.1421356237 );
vf.addControlPoint( 44850.7061568 , 50684.2855885 , 5.8137767415 , 11.627553483 );
vf.addControlPoint( 17141.2038566 , 47767.4958726 , 5.8137767415 , 11.627553483 );
vf.addControlPoint( 22245.5858592 , 48861.2920161 , 14.1421356237 , 14.1421356237 );
vf.addControlPoint( 52871.8778753 , 47038.2984437 , 7.6026311235 , 15.205262247 );
vf.addControlPoint( 70737.2148847 , 25891.573004 , 8.4970583145 , 16.994116629 );

vf.updateVectorField()
```

Functions which can be used in python script:
```
init: initialize grid (minx, minz, maxx, maxz, cellsize, height)
setVisible(bool v)
toggleVisible() 
setPointScale(float ps): change point size
setArrowScale(float as): change arrow size
nextParticleType(): particles can be displayed as points or arrows
loadElevationFromFile(string txtfile): the elevation values can be used to adjust particles' height
setElevationScale(float es): change scale of elevation values
```

## Standalone app

Tested with MacOS 10.12.4

```
cd vectorfield/app
mkdir build
cd build
cmake .. (or cmake .. -G Xcode to build XCode project)
make (or run build in XCode)
```
