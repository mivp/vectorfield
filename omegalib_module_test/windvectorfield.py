import vectorfield
from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

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

vf.setPointScale(0.8)
vf.updateVectorField()

# model
scene = getSceneManager()

# camera
cam = getDefaultCamera()
cam.getController().setSpeed(10000)
cam.setPosition(Vector3(0, 0, 0))
setNearFarZ(2, 400000)

#menu
mm = MenuManager.createAndInitialize()
menu = mm.getMainMenu()
mm.setMainMenu(menu)
campos = [39286.29, 64904.98, 125220.43]
camori = [-0.94, 0.33, 0.09, 0.04]
cmd = 'cam.setPosition(Vector3(' + str(campos[0]) + ',' + str(campos[1]) + ',' + str(campos[2]) + ')),' + \
		'cam.setOrientation(Quaternion(' + str(camori[0]) + ',' + str(camori[1]) + ',' + str(camori[2]) + ',' + str(camori[3]) + '))'
menu.addButton("Go to camera 1", cmd)
