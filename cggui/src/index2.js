// Load the rendering pieces we want to use (for both WebGL and WebGPU)
import '@kitware/vtk.js/Rendering/Profiles/Volume';
import '@kitware/vtk.js/favicon';

// Load the rendering pieces we want to use (for both WebGL and WebGPU)
//import '@kitware/vtk.js/Rendering/Profiles/Geometry';

import vtkFullScreenRenderWindow from '@kitware/vtk.js/Rendering/Misc/FullScreenRenderWindow';
import vtkActor from '@kitware/vtk.js/Rendering/Core/Actor';
import vtkMapper from '@kitware/vtk.js/Rendering/Core/Mapper';

import vtkColorTransferFunction from '@kitware/vtk.js/Rendering/Core/ColorTransferFunction';
import vtkDataArray from '@kitware/vtk.js/Common/Core/DataArray';
import vtkImageData from '@kitware/vtk.js/Common/DataModel/ImageData';
import vtkPiecewiseFunction from '@kitware/vtk.js/Common/DataModel/PiecewiseFunction';
import vtkRenderer from '@kitware/vtk.js/Rendering/Core/Renderer';
import vtkVolume from '@kitware/vtk.js/Rendering/Core/Volume';
import vtkVolumeMapper from '@kitware/vtk.js/Rendering/Core/VolumeMapper';
import vtkImageMapper from '@kitware/vtk.js/Rendering/Core/ImageMapper';
import vtkImageSlice from '@kitware/vtk.js/Rendering/Core/ImageSlice';
import vtkPlane from '@kitware/vtk.js/Common/DataModel/Plane';
import Constants from '@kitware/vtk.js/Rendering/Core/ImageMapper/Constants';
import vtkXMLImageDataReader from '@kitware/vtk.js/IO/XML/XMLImageDataReader';
import vtkInteractorStyleManipulator from '@kitware/vtk.js/Interaction/Style/InteractorStyleManipulator';
import vtkMouseCameraTrackballPanManipulator from '@kitware/vtk.js/Interaction/Manipulators/MouseCameraTrackballPanManipulator';
import vtkMouseCameraTrackballRollManipulator from '@kitware/vtk.js/Interaction/Manipulators/MouseCameraTrackballRollManipulator';
import vtkMouseCameraTrackballRotateManipulator from '@kitware/vtk.js/Interaction/Manipulators/MouseCameraTrackballRotateManipulator';
import vtkMouseCameraTrackballZoomManipulator from '@kitware/vtk.js/Interaction/Manipulators/MouseCameraTrackballZoomManipulator';
import vtkGestureCameraManipulator from '@kitware/vtk.js/Interaction/Manipulators/GestureCameraManipulator';
import vtkColorMaps from '@kitware/vtk.js/Rendering/Core/ColorTransferFunction/ColorMaps';
import vtkOrientationMarkerWidget from '@kitware/vtk.js/Interaction/Widgets/OrientationMarkerWidget';
import vtkAnnotatedCubeActor from '@kitware/vtk.js/Rendering/Core/AnnotatedCubeActor';

import controlPanel from './controlPanel.html';

// ----------------------------------------------------------------------------
// Standard rendering code setup
// ----------------------------------------------------------------------------

const fullScreenRenderer = vtkFullScreenRenderWindow.newInstance({
  background: [0.8, 0.8, 0.8],
});
fullScreenRenderer.addController(controlPanel);
fullScreenRenderer.getControlContainer().style.width="40%" ;
const renderer = fullScreenRenderer.getRenderer();
const renderWindow = fullScreenRenderer.getRenderWindow();

// ----------------------------------------------------------------------------
// Example code
// ----------------------------------------------------------------------------

// create cone
// const coneSource = vtkConeSource.newInstance();
// const actor = vtkActor.newInstance();
// const mapper = vtkMapper.newInstance();
// 
// actor.setMapper(mapper);
//mapper.setInputConnection(coneSource.getOutputPort());

//renderer.addActor(actor);

// create axes
const axes = vtkAnnotatedCubeActor.newInstance();
axes.setDefaultStyle({
  text: '+X',
  fontStyle: 'bold',
  fontFamily: 'Arial',
  fontColor: 'black',
  fontSizeScale: (res) => res / 2,
  faceColor: '#0000ff',
  faceRotation: 0,
  edgeThickness: 0.1,
  edgeColor: 'black',
  resolution: 400,
});
// axes.setXPlusFaceProperty({ text: '+X' });
axes.setXMinusFaceProperty({
  text: '-X',
  faceColor: '#ffff00',
  faceRotation: 90,
  fontStyle: 'italic',
});
axes.setYPlusFaceProperty({
  text: '+Y',
  faceColor: '#00ff00',
  fontSizeScale: (res) => res / 4,
});
axes.setYMinusFaceProperty({
  text: '-Y',
  faceColor: '#00ffff',
  fontColor: 'white',
});
axes.setZPlusFaceProperty({
  text: '+Z',
  edgeColor: 'yellow',
});
axes.setZMinusFaceProperty({ text: '-Z', faceRotation: 45, edgeThickness: 0 });

// create orientation widget
const orientationWidget = vtkOrientationMarkerWidget.newInstance({
  actor: axes,
  interactor: renderWindow.getInteractor(),
});
orientationWidget.setEnabled(true);
orientationWidget.setViewportCorner(
  vtkOrientationMarkerWidget.Corners.BOTTOM_RIGHT
);
orientationWidget.setViewportSize(0.15);
orientationWidget.setMinPixelSize(100);
orientationWidget.setMaxPixelSize(300);

renderer.resetCamera();
renderWindow.render();
