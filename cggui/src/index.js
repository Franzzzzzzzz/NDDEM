// Load the rendering pieces we want to use (for both WebGL and WebGPU)
import '@kitware/vtk.js/Rendering/Profiles/Volume';
import '@kitware/vtk.js/Rendering/Profiles/Geometry';

import vtkColorTransferFunction from '@kitware/vtk.js/Rendering/Core/ColorTransferFunction';
import vtkDataArray from '@kitware/vtk.js/Common/Core/DataArray';
import vtkFullScreenRenderWindow from '@kitware/vtk.js/Rendering/Misc/FullScreenRenderWindow';
import vtkImageData from '@kitware/vtk.js/Common/DataModel/ImageData';
import vtkPiecewiseFunction from '@kitware/vtk.js/Common/DataModel/PiecewiseFunction';
import vtkRenderer from '@kitware/vtk.js/Rendering/Core/Renderer';
import vtkActor from '@kitware/vtk.js/Rendering/Core/Actor';
import vtkVolume from '@kitware/vtk.js/Rendering/Core/Volume';
import vtkMapper from '@kitware/vtk.js/Rendering/Core/Mapper';
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
import vtkAxesActor from '@kitware/vtk.js/Rendering/Core/AxesActor';
import vtkSphereSource from '@kitware/vtk.js/Filters/Sources/SphereSource';

//import './popup.css'
import './popupform.css'
import controlPanel from './controlPanel.html';
import { VtkDataTypes } from '@kitware/vtk.js/Common/Core/DataArray/Constants'
import noUiSlider from 'nouislider';
import './nouislider.css';
import wNumb from 'wnumb';

// ----------------------------------------------------------------------------
// Standard rendering code setupCoarseGraining_Liggghts.js
// ----------------------------------------------------------------------------

const fullScreenRenderer = vtkFullScreenRenderWindow.newInstance({
  background: [0.8, 0.8, 0.8],
});
const renderer = fullScreenRenderer.getRenderer();
const renderWindow = fullScreenRenderer.getRenderWindow();

fullScreenRenderer.addController(controlPanel);
fullScreenRenderer.getControlContainer().style.width="40%" ;

var dispts = 0 ;
var defaultdensity = -1 ; 
var sphereinfos=false ;
var actor2=[] ;

var worker = new Worker("worker.js");

// ----------------------------------------------------------------------------
// Example code
// ----------------------------------------------------------------------------
// Server is not sending the .gz and with the compress header
// Need to fetch the true file name and uncompress it locally
// ----------------------------------------------------------------------------

const reader = vtkXMLImageDataReader.newInstance();

const actor = vtkVolume.newInstance();
const mapper = vtkVolumeMapper.newInstance();
mapper.setSampleDistance(1.1);
actor.setMapper(mapper);

const clipPlane1 = vtkPlane.newInstance();
let clipPlane1Position = 0;
let clipPlane1RotationAngle = 0;
let clipPlane2RotationAngle = 0;
const clipPlane1Normal = [-1, 1, 0];
const rotationNormal = [0, 1, 0];

// create color and opacity transfer functions
//const ctfun = vtkColorTransferFunction.newInstance();
//ctfun.addRGBPoint(0, 0, 0, 0);
//ctfun.addRGBPoint(255, 1.0, 1.0, 1.0);
var lookupTable = vtkColorTransferFunction.newInstance();
var preset = vtkColorMaps.getPresetByName('Inferno (matplotlib)');
lookupTable.applyColorMap(preset);

lookupTable.setNanColor(1.0, 1.0, 1.0, 1.0);    // should be after applyColorMap
lookupTable.build();
lookupTable.setMappingRange(...[0,256]);
lookupTable.updateRange();

actor.getProperty().setRGBTransferFunction(0, lookupTable);
actor.getProperty().setScalarOpacityUnitDistance(0, 3.0);
//actor.getProperty().setInterpolationTypeToLinear();
actor.getProperty().setUseGradientOpacity(0, false);
actor.getProperty().setShade(false);
actor.getProperty().setAmbient(1);
actor.getProperty().setDiffuse(0.7);
actor.getProperty().setSpecular(0.3);
actor.getProperty().setSpecularPower(8.0);
//actor.getProperty().setInterpolationTypeToFastLinear() ; 
console.log(actor.getProperty())

var width = 3, height = 3, depth = 3;
var size = width * height * depth;

var values = [0,255,0,255,255,255,0,255,0, 255,255,255,255,0,255,255,255,255, 0,255,0,255,255,255,0,255,0];
// for (var i = 0; i < size; i++) {
//     values[i] = i/26*255;
// }
var values = [256,0,256,0,256,0,256,0,256, 0,256,0,256,0,256,0,256,0,  256,0,256,0,256,0,256,0,256];
//var values=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,256,0,0,0,0,0,256,256,256]
lookupTable.setMappingRange(...[0,256]);
lookupTable.updateRange();

var scalars = vtkDataArray.newInstance({
    values: values,
    numberOfComponents: 1, // number of channels (grayscale)
    dataType: VtkDataTypes.FLOAT, // values encoding
    name: 'scalars'
});

var imageData = vtkImageData.newInstance();
imageData.setOrigin(0, 0, 0);
imageData.setSpacing(1, 1, 1);
imageData.setExtent(0, width - 1, 0, height - 1, 0, depth - 1);
imageData.getPointData().setScalars(scalars);

mapper.setInputData(imageData);
renderer.addVolume(actor);


//---- Manipulators
const interactor = renderWindow.getInteractor();
interactor.setDesiredUpdateRate(15.0);
const interactorStyle = vtkInteractorStyleManipulator.newInstance();

interactorStyle.removeAllMouseManipulators();
const manipulator1 = vtkMouseCameraTrackballPanManipulator.newInstance() ;
manipulator1.setButton(2) ; 
interactorStyle.addMouseManipulator(manipulator1);
const manipulator2 = vtkMouseCameraTrackballZoomManipulator.newInstance(); //scroll
manipulator2.setButton(2) ;
manipulator2.setScrollEnabled(true) ; 
interactorStyle.addMouseManipulator(manipulator2);
const manipulator3 =vtkMouseCameraTrackballRollManipulator.newInstance(); //shift + left
manipulator3.setButton(1) ; 
manipulator3.setShift(true) ; 
interactorStyle.addMouseManipulator(manipulator3);
const manipulator4 =vtkMouseCameraTrackballRotateManipulator.newInstance(); //shift + middle
manipulator4.setButton(1) ; 
interactorStyle.addMouseManipulator(manipulator4);
interactorStyle.addGestureManipulator(vtkGestureCameraManipulator.newInstance());

interactorStyle.setCenterOfRotation(1,1,1) ; 
console.log(interactorStyle.getCenterOfRotation()) ; 
renderWindow.getInteractor().setInteractorStyle(interactorStyle);

// create axes
// const axes = vtkAnnotatedCubeActor.newInstance();
// axes.setDefaultStyle({
//   text: 'X+',
//   fontStyle: 'bold',
//   fontFamily: 'Arial',
//   fontColor: 'black',
//   fontSizeScale: (res) => res / 2,
//   faceColor: '#ff0000',
//   faceRotation: 0,
//   edgeThickness: 0.1,
//   edgeColor: 'black',
//   resolution: 400,
// });
// axes.setXPlusFaceProperty ({ text: 'X+', faceRotation: 270 });
// axes.setXMinusFaceProperty({ text: 'X-', faceColor: '#ff0000', faceRotation: 90});
// axes.setYPlusFaceProperty ({ text: 'Y+', faceColor: '#ffff00', faceRotation: 0});
// axes.setYMinusFaceProperty({ text: 'Y-', faceColor: '#ffff00', faceRotation: 180});
// axes.setZPlusFaceProperty ({ text: 'Z+', faceColor: '#00ff00'});
// axes.setZMinusFaceProperty({ text: '-Z', faceColor: '#00ff00'});
const axes = vtkAxesActor.newInstance();
// create orientation widget
const orientationWidget = vtkOrientationMarkerWidget.newInstance({
  actor: axes,
  interactor: renderWindow.getInteractor(),
});
orientationWidget.setEnabled(true);
orientationWidget.setViewportCorner(
  vtkOrientationMarkerWidget.Corners.BOTTOM_LEFT
);
orientationWidget.setViewportSize(0.15);
orientationWidget.setMinPixelSize(100);
orientationWidget.setMaxPixelSize(300);


clipPlane1.setNormal([1,0,0]);
clipPlane1.setOrigin([0,0,0]);
mapper.addClippingPlane(clipPlane1);

renderer.resetCamera();
renderer.getActiveCamera().elevation(70);
renderer.getActiveCamera().setViewUp(0,0,1)
orientationWidget.updateMarkerOrientation()
renderWindow.render();

//===============================================================
// Gui handling
//===============================================================
//---------- Collapsibles
var coll = document.getElementsByClassName("collapsible");
var i;
for (i = 0; i < coll.length; i++) {
  coll[i].addEventListener("click", function() {
    this.classList.toggle("active");
    var content = this.nextElementSibling;
    if (content.style.display === "block") {
      content.style.display = "none";
    } else {
      content.style.display = "block";
    }
  });
} 

// ---------- time multi-slider
var timeslider=document.getElementById('timeslider') ; 
noUiSlider.create(timeslider, {
    start: [20, 80],
    connect: true,
    range: {
        'min': 0,
        'max': 100
    },
    step: 1,
    format: wNumb({decimals: 0}),
    tooltips: true,
    pips: {
        mode: 'range',
        density: 5
    }
});
var dispslider=document.getElementById('dispslider') ; 
noUiSlider.create(dispslider, {
    start: [0],
    connect: true,
    range: {
        'min': 0,
        'max': 0
    },
    step: 1,
    format: wNumb({decimals: 0}),
    tooltips: true,
    pips: {
        mode: 'range',
        density: 5
    }
});
var lst_param = ['RHO', 'TotalStress', 'Pressure', 'KineticPressure', 'ShearStress', 'VolumetricStrainRate', 'ShearStrainRate', 'RotationalVelocity', 'RotationalVelocityMag', "EKT", "zT", "VAVG", "EKR", "zR", "TC", "eKT", "qTK", "TK", "eKR", "qTC", "StrainRate", "ROT", "mC", "MC", "qRK", "MK", "qRC", "I", 
"window", "windowsize", "xgrid", "ygrid", "zgrid", "xmin", "xmax", "xper", "ymin", "ymax", "yper", "zmin", "zmax", "zper", "timeaverage", "timeslider"] ; 
//==============================================================================
//--------------------- File handling ------------------------------------------
var curfile = 0 ; 
var isedit = false ; 
document.querySelector('.addfile').addEventListener('click', async () => {
    curfile ++ ; 
    document.getElementById("addfileform").style.display = "block";
    document.getElementById("filename"+String(curfile)).hidden = false ; 
});

function filenameupdate (filelist)
{
    document.getElementById("updatefile").style.visibility = "visible";
    document.getElementById("filetype"+String(curfile)).hidden=false ; 
    let pattern=null ; 
    if (filelist.length>1)
    {
        document.getElementById("file_multifile").hidden=false ; 
        let numbers = [] ; 
        for (let i=0 ; i<filelist.length ; i++)
        {
            const matches = filelist[i].name.match(/\d+(\.\d+)?/g); // Extract numbers from file name
            // If numbers are found
            if (matches && matches.length > 0) 
            {
                numbers[i]=matches.map(Number)[matches.length -1] ; 
                console.log(filelist[0].name, String(numbers[i])) ; 
                if (pattern === null) {
                    if (matches[matches.length-1].includes(".")) pattern="%f" ;
                    else pattern="%d" ; 
                    document.getElementById("file_pattern").value = filelist[0].name.replace(String(numbers[i]), pattern) ; 
                }
            }
            else
                console.log("Invalid file selection") ; 
        }
        // Try to identify filetype
        const matches =filelist[0].name.match(/vtu/g);
        if (matches && matches.length > 0) 
        {
            document.getElementById("filetype"+String(curfile)).selectedIndex=1 ;
        }
        document.getElementById("file_initial").value=Math.min(...numbers) ; 
        document.getElementById("file_delta").value=(Math.max(...numbers)-Math.min(...numbers))/(filelist.length-1); 
    }
    else
        document.getElementById("file_pattern").value=filelist[0].name ; 
}

document.getElementById("filename1").addEventListener('change', async() =>
{ 
    var filelist = document.getElementById("filename1").files ;
    filenameupdate(filelist) ; 
});
document.getElementById("filename2").addEventListener('change', async() =>
{ 
    var filelist = document.getElementById("filename2").files ;
    filenameupdate(filelist) ; 
});

document.getElementById("cancelfile").addEventListener('click', async() =>
{
    //document.getElementById("filename"+String(curfile)).value="" ; 
    document.getElementById("filetype"+String(curfile)).hidden=true ; 
    document.getElementById("file_multifile").hidden=true ;
    document.getElementById("file_delta").value="" ; 
    document.getElementById("file_initial").value="" ;
    document.getElementById("file_pattern").value="" ; 
    document.getElementById("updatefile").style.visibility = "hidden";
    document.getElementById("filename1").hidden=true ; 
    document.getElementById("filename2").hidden=true ; 
    document.getElementById("addfileform").style.display = "None";
    curfile -- ; 
}) ; 

document.getElementById("updatefile").addEventListener('click', async() => {
    var name = "file" + String(curfile) ; 
    document.getElementById(name+"_number").innerHTML = document.getElementById("filename"+String(curfile)).files.length + " file(s)" ; 
    document.getElementById(name+"_type").value=document.getElementById("filetype"+String(curfile)).value ; 
    document.getElementById(name+"_delta").value=document.getElementById("file_delta").value ; 
    document.getElementById(name+"_initial").value=document.getElementById("file_initial").value ;
    document.getElementById(name+"_pattern").value=document.getElementById("file_pattern").value ;
    if (document.getElementById("filename"+String(curfile)).files.length>1)
    {
        document.getElementById(name+"_delta").hidden=false ; 
        document.getElementById(name+"_initial").hidden=false ;
        document.getElementById(name+"_delta_label").hidden=false ; 
        document.getElementById(name+"_initial_label").hidden=false ;
    }
    else
    {
        document.getElementById(name+"_delta").hidden=true ; 
        document.getElementById(name+"_initial").hidden=true ;
        document.getElementById(name+"_delta_label").hidden=true ; 
        document.getElementById(name+"_initial_label").hidden=true ;
    }
    document.getElementById(name+"_info").hidden=false ; 
    if (curfile==1)
        document.getElementById("remove"+name).visible==true ; 
    else 
        document.getElementById("remove"+name).visible==false ; 
    document.getElementById("addfileform").style.display = "None"; 
    document.getElementById("filename"+String(curfile)).hidden=true ; 
    document.getElementById("filetype"+String(curfile)).hidden=true ; 
    document.getElementById("updatefile").style.visibility="hidden" ; 
    
    if (curfile==1)
    {
        if (document.getElementById("filetype1").value==3) document.querySelector('.addfile').hidden=true ; 
        else 
        {
            document.querySelector('.addfile').hidden=false ; 
            document.querySelector('.addfile').innerHTML="Add contact file" ; 
        }        
    }
    
    var data=build_file_json() ;
    for(i=0 ; i<data["file"].length ; i++) data["file"][i]["action"]="donothing" ; 
    if (isedit) data.file[curfile-1]["action"]="edit" ; 
    else data["file"][curfile-1]["action"]="create" ; 
    
    // Liggghts contact only
    if (data["file"].length>1)
        data["file"][1].mapping={"id1": "c_cout[1]", "id2": "c_cout[2]", "per": "c_cout[3]", "fx": "c_cout[4]", "fy": "c_cout[5]", "fz": "c_cout[6]"} ;
    
    console.log(data)
    document.getElementById("wait").hidden=false ; 
    worker.postMessage([ 'initialise', data, document.getElementById("filename1").files, document.getElementById("filename2").files])
    console.log("HERE") 
    isedit=false ;
}) ; 

function fileformedit() 
{
    var name = "file" + String(curfile) ;
    document.getElementById("filename"+String(curfile)).hidden=false ; 
    document.getElementById("filetype"+String(curfile)).value=document.getElementById(name+"_type").value ; 
    if (document.getElementById(name+"_delta").hidden == false)
    {
        document.getElementById("file_multifile").hidden=false ;
        document.getElementById("file_delta").value=document.getElementById(name+"_delta").value ; 
        document.getElementById("file_initial").value=document.getElementById(name+"_initial").value ;
        document.getElementById("file_pattern").value=document.getElementById(name+"_pattern").value ;
    }
    document.getElementById("updatefile").hidden=false ;
}
document.getElementById("editfile1").addEventListener('click', async() => {
    curfile = 1 ; isedit=true ; 
    fileformedit() ; 
    document.getElementById("addfileform").style.display = "block"; 
}) ; 
document.getElementById("editfile2").addEventListener('click', async() => {
    curfile = 2 ; isedit=true ;
    fileformedit() ; 
    document.getElementById("addfileform").style.display = "block"; 
}) ; 

document.getElementById("drop_zone").addEventListener('drop', async(ev) => {
    console.log("File(s) dropped");
    ev.preventDefault();

  if (ev.dataTransfer.items) {
    // Use DataTransferItemList interface to access the file(s)
    [...ev.dataTransfer.items].forEach((item, i) => {
      // If dropped items aren't files, reject them
      if (item.kind === "file") {
        const file = item.getAsFile();
        console.log(`… file[${i}].name = ${file.name}`);
      }
    });
  } else {
    // Use DataTransfer interface to access the file(s)
    [...ev.dataTransfer.files].forEach((file, i) => {
      console.log(`… file[${i}].name = ${file.name}`);
    });
  }
  document.getElementById("indrop").hidden=true ; 
}) ; 
document.getElementById("drop_zone").addEventListener('dragover', async(ev) => {
ev.preventDefault();
document.getElementById("indrop").hidden=false ; 
}) ; 
document.getElementById("drop_zone").addEventListener('dragleave', async(ev) => {
ev.preventDefault();
document.getElementById("indrop").hidden=true ; 
}) ; 

//-----
var ncolumnmapping=0 ; 
/*document.getElementById('addcolumn').addEventListener('click', async () => {
    ncolumnmapping++ ; 
    var fragment = document.createDocumentFragment();
    var l0 = document.createElement("br")
    fragment.appendChild(l0) ; l0.id="columnspacing"+ncolumnmapping ;
    var l1 = document.createElement("input")
    l1.type="text" ; l1.id="columnname"+ncolumnmapping ; l1.placeholder="Column name"
    fragment.appendChild(l1) ; 
    var l01 = document.createElement("span")
    l01.textContent="=>" ; l01.id="columnseparator"+ncolumnmapping ; 
    fragment.appendChild(l01) ; 
    
    var l2 = document.createElement("select")
    l2.id="columnselect"+ncolumnmapping ;
    fragment.appendChild(l2) ; 
    
    var possiblecolumn = ["lpq", "fpq_x", "fpq_y", "fpq_z"] ; 
    possiblecolumn.forEach( name => {
        var l3 = document.createElement("option")
        l3.textContent=name ; 
        l2.options.add(l3)
    }) ; 
    var l4 = document.createElement("button")
    l4.textContent="X";  l4.id="columnbutton"+ncolumnmapping ;
    var l44 = ncolumnmapping ; 
    l4.addEventListener('click', async() => {
        document.getElementById("columnspacing"+l44).remove();
        document.getElementById("columnseparator"+l44).remove() ; 
        document.getElementById("columnname"+l44).remove();
        document.getElementById("columnselect"+l44).remove();
        document.getElementById("columnbutton"+l44).remove() ; 
    }) ;
    fragment.appendChild(l4) ; 
    console.log(fragment) ; 
    
    
    document.getElementById('columnmapping').appendChild(fragment) ;
});*/
//======================= Handle change in any of the parameters ====================================================
document.getElementById('RHO').addEventListener('change', async () => {
    if (document.getElementById('RHO').checked==false)
        alert('The density is needed for most field calculation. It is strongly recommended to keep it checked') ; 
});
function setstresseschecked() {
        document.getElementById('TC').checked=true ; 
        document.getElementById('TK').checked=true ; 
}
function setvelocitychecked() { document.getElementById('VAVG').checked=true ; }
document.getElementById('TotalStress').addEventListener('change', async () => { if (document.getElementById('TotalStress').checked==true) setstresseschecked() ; update_parameters ()});
document.getElementById('Pressure').addEventListener('change', async () => { if (document.getElementById('Pressure').checked==true) setstresseschecked() ; update_parameters ()});
document.getElementById('KineticPressure').addEventListener('change', async () => {if (document.getElementById('KineticPressure').checked==true) setstresseschecked() ; update_parameters ()});
document.getElementById('ShearStress').addEventListener('change', async () => { if (document.getElementById('ShearStress').checked==true) setstresseschecked() ; update_parameters ()});
document.getElementById('VolumetricStrainRate').addEventListener('change', async () => { if (document.getElementById('VolumetricStrainRate').checked==true) setvelocitychecked() ; update_parameters () });
document.getElementById('ShearStrainRate').addEventListener('change', async () => { if (document.getElementById('ShearStrainRate').checked==true) setvelocitychecked() ; update_parameters ()});
document.getElementById('RotationalVelocity').addEventListener('change', async () => { if (document.getElementById('RotationalVelocity').checked==true) setvelocitychecked() ; update_parameters () });
document.getElementById('RotationalVelocityMag').addEventListener('change', async () => { if (document.getElementById('RotationalVelocityMag').checked==true) setvelocitychecked() ; update_parameters ()});


var lst_param = ['RHO', 'TotalStress', 'Pressure', 'KineticPressure', 'ShearStress', 'VolumetricStrainRate', 'ShearStrainRate', 'RotationalVelocity', 'RotationalVelocityMag', "EKT", "zT", "VAVG", "EKR", "zR", "TC", "eKT", "qTK", "TK", "eKR", "qTC", "StrainRate", "ROT", "mC", "MC", "qRK", "MK", "qRC", "I", 
"window", "windowsize", "xgrid", "ygrid", "zgrid", "xmin", "xmax", "xper", "ymin", "ymax", "yper", "zmin", "zmax", "zper", "timeaverage", "timeslider"] ; 
for (var i=0; i<lst_param.length; i++)
{
    document.getElementById(lst_param[i]).addEventListener('change', async () => {update_parameters ()}) ; 
}
/*document.getElementById("EKT" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("zT"  ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("VAVG").addEventListener('change', async () => {update_parameters()}) 
document.getElementById("EKR" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("zR"  ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("TC"  ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("eKT" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("qTK" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("TK"  ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("eKR" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("qTC" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("StrainRate").addEventListener('change', async () => {update_parameters()}) 
document.getElementById("ROT").addEventListener('change', async () => {update_parameters()}) 
document.getElementById("mC" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("MC" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("qRK").addEventListener('change', async () => {update_parameters()}) 
document.getElementById("MK" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("qRC").addEventListener('change', async () => {update_parameters()}) 
document.getElementById("I"  ).addEventListener('change', async () => {update_parameters()}) 

document.getElementById("window"     ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("windowsize" ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("xgrid"      ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("ygrid"      ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("zgrid"      ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("xmin"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("xmax"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("xper"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("ymin"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("ymax"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("yper"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("zmin"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("zmax"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("zper"       ).addEventListener('change', async () => {update_parameters()}) 
document.getElementById("timeaverage").addEventListener('change', async () => {update_parameters()}) 
document.getElementById("timeslider" ).addEventListener('change', async () => {update_parameters()}) */
// ========================================= BUILDING THE JSON data ========================================
function build_file_json() 
{
    var data = {}    
    var files = []
    
    var naddedfiles = 0; 
    if (document.getElementById("file2_info").hidden==false) naddedfiles=2 ;
    else if (document.getElementById("file1_info").hidden==false) naddedfiles=1 ;
    
    for (var fic = 1 ; fic <=naddedfiles ; fic++)
    {
        files[fic-1] = {} ;
        
        files[fic-1]["filename"] = document.getElementById("file"+String(fic)+"_pattern").value ; 
        files[fic-1]["initial"] = Number(document.getElementById("file"+String(fic)+"_initial").value) ; 
        files[fic-1]["delta"] = Number(document.getElementById("file"+String(fic)+"_delta").value) ; 
        
        var filetype = document.getElementById("file"+String(fic)+"_type").value ; 
        if (filetype=="LiggghtsParticles")
        {
            files[fic-1]["content"] = "particles" ; 
            files[fic-1]["format"] = "liggghts" ; 
        }
        else if (filetype == "MercuryVTUParticles")
        {
            files[fic-1]["content"] = "particles" ; 
            files[fic-1]["format"] = "mercury_vtu" ; 
        }
        else if (filetype == "Yade")
        {
            files[fic-1]["content"] = "particles" ; 
            files[fic-1]["format"] = "yade" ; 
        }
        else if (filetype == "LiggghtsContacts")
        {
            files[fic-1]["content"] = "contacts" ; 
            files[fic-1]["format"] = "liggghts" ; 
        }
        else if (filetype == "MercuryVTUContacts")
        {
            files[fic-1]["content"] = "contacts" ; 
            files[fic-1]["format"] = "mercury_vtu" ; 
        }
        else if (filetype == "NDDEM")
        {
            files[fic-1]["content"] = "both" ; 
            files[fic-1]["format"] = "NDDEM" ; 
        }        
        else
            console.log("This should never happen: unknown file format") ; 
        
        /*if (ncolumnmapping>0 && fic==2)
        {
            files[fic-1]["mapping"]={} ; 
            for (var column=1 ; column <= ncolumnmapping ; column++) 
            {
                try {
                    files[fic-1]["mapping"][document.getElementById("columnselect"+column).value] = document.getElementById("columnname"+column).value ; 
                } catch (e) {continue ;}
            }
        }*/
    }
    data["file"]=files ; 
    return data ; 
}

function build_parameter_json()
{
    var data = {}
    data["boxes"] = [parseInt(document.getElementById("xgrid").value), parseInt(document.getElementById("ygrid").value), parseInt(document.getElementById("zgrid").value)] ;
    data["boundaries"] = [[parseFloat(document.getElementById("xmin").value), parseFloat(document.getElementById("ymin").value), parseFloat(document.getElementById("zmin").value)], 
                          [parseFloat(document.getElementById("xmax").value), parseFloat(document.getElementById("ymax").value), parseFloat(document.getElementById("zmax").value)]] ;
    data["periodicity"] = [document.getElementById("xper").checked, document.getElementById("yper").checked, document.getElementById("zper").checked] ;
                          
    data["window size"] = parseFloat(document.getElementById("windowsize").value) ; 
    data["window"] =document.getElementById("window").value ;
    
    var times=timeslider.noUiSlider.get() ; 
    data["skip"]=parseInt(times[0]) ; 
    data["max time"]=parseInt(times[1])-parseInt(times[0]) ;
    
    if (data["skip"]!=parseInt(dispslider.noUiSlider.options.range.min) || data["skip"]+data["max time"]-1!=parseInt(dispslider.noUiSlider.options.range.max))
    {
        dispslider.noUiSlider.updateOptions({range: {'min':  data["skip"], 'max': data["skip"]+data["max time"]-1}}) ; 
        dispslider.noUiSlider.set(data["skip"]) ; 
    }
    
    data["time average"]=document.getElementById("timeaverage").value ; 
    
    var fieldlst= ["RHO", "I", "VAVG",  "TC", "TK", "ROT", "MC", "MK" , "mC", "EKT", "eKT", "EKR", "eKR", "qTC", "qTK", "qRC", "qRK", "zT" , "zR", "TotalStress", "Pressure", "KineticPressure", "ShearStress", "StrainRate", "VolumetricStrainRate", "ShearStrainRate", "RotationalVelocity", "RotationalVelocityMag"] ;    
    data["fields"]=[]
    fieldlst.forEach(name => {if (document.getElementById(name).checked==true) data["fields"].push(name);}) ; 
        
    if (defaultdensity != -1) data["density"]=defaultdensity ; 
    
    return data ; 
}

function build_save_json()
{
    var data={}
    var select = document.getElementById('saveformat');
    data["saveformat"] = [...document.getElementById('saveformat').selectedOptions].map(option => option.value);
    data["save"] = document.getElementById('savefile').value; 
    
    return data ; 
}


function generateallparameters () {
    var data = {}

    var d1=build_file_json() ; 
    var d2=build_parameter_json() ;
    var d3=build_save_json() ; 
    data = Object.assign({}, d1, d2, d3);
    
    console.log(data) ; 
    return (data) ; 
} ; 
document.querySelector('.savejson').addEventListener('click', async () => {
    var data=generateallparameters() ; 
    var content=JSON.stringify(data, null, 2);
    var a = document.createElement("a");
    var file = new Blob([content], {type: "application/json"});
    a.href = URL.createObjectURL(file);
    a.download = "CoarseGrainingParameters.json";
    a.click();
    
});
//-------------------
document.querySelector('.loadjson').addEventListener('click', async () => {
   document.getElementById('loadjsonfile').click() ;  
});

document.getElementById('loadjsonfile').addEventListener('change', function() {
    var fr=new FileReader();
    fr.readAsText(this.files[0]);
    fr.onload=function(){
        var data = JSON.parse(fr.result);
        console.log(data) ; 
        document.getElementById("xgrid").value = data["boxes"][0] ; 
        document.getElementById("ygrid").value = data["boxes"][1] ; 
        document.getElementById("zgrid").value = data["boxes"][2] ; 
        document.getElementById("xmin").value = data["boundaries"][0][0] ; 
        document.getElementById("ymin").value = data["boundaries"][0][1] ; 
        document.getElementById("zmin").value = data["boundaries"][0][2] ; 
        document.getElementById("xmax").value = data["boundaries"][1][0] ; 
        document.getElementById("ymax").value = data["boundaries"][1][1] ; 
        document.getElementById("zmax").value = data["boundaries"][1][2] ; 
        document.getElementById("xper").checked = data["periodicity"][0] ; 
        document.getElementById("yper").checked = data["periodicity"][1] ; 
        document.getElementById("zper").checked = data["periodicity"][2] ; 
        document.getElementById("windowsize").value = data["window size"] ;
        document.getElementById("window").value = data["window"] ;
        
        timeslider.noUiSlider.updateOptions({range: {'min': 0,'max': data["skip"]+data["max time"]}}) ; 
        timeslider.noUiSlider.set([data["skip"], data["skip"]+data["max time"]]) ;
        document.getElementById("timeaverage").value = data["time average"] ;
        
        dispslider.noUiSlider.updateOptions({range: {'min':  data["skip"], 'max': data["skip"]+data["max time"]-1}}) ; 
        dispslider.noUiSlider.set(data["skip"]) ; 
        
        if (data["density"])
            defaultdensity=data["density"] ; 
    
        var fieldlst= ["RHO", "I", "VAVG",  "TC", "TK", "ROT", "MC", "MK" , "mC", "EKT", "eKT", "EKR", "eKR", "qTC", "qTK", "qRC", "qRK", "zT" , "zR", "TotalStress", "Pressure", "KineticPressure", "ShearStress", "StrainRate", "VolumetricStrainRate", "ShearStrainRate", "RotationalVelocity", "RotationalVelocityMag"] ;    
        fieldlst.forEach(name => document.getElementById(name).checked = false) ; 
        data["fields"].forEach(name => document.getElementById(name).checked = true)
        /* TODO
        * var select = document.getElementById('saveformat');
        data["saveformat"] = [...document.getElementById('saveformat').selectedOptions].map(option => option.value);
        data["save"] = document.getElementById('savefile').value; */

    }}) ; 
//------------------

var initialised=false ; 
var updated=false

function update_parameters ()
{
    var data=build_parameter_json() 
    worker.postMessage(['setparameters', data]) ;
}

/*document.getElementById('applyparam').addEventListener('click', async () => {
    var data=generateallparameters() ;
    if (initialised == false)
        alert("You need first to update the file(s) in the first tab.") ; 
    else
    updated=false ; 
});*/

document.getElementById('updateCG').addEventListener('click', async () => {
    var ts=parseInt(dispslider.noUiSlider.get()) ; 
    worker.postMessage(['processts', ts]) ;
});

document.getElementById('displayedfield').addEventListener('change', async () => {
    var fieldlist = {"RHO": 0, "I":0 , "VAVG": 3,  "TC": 9, "TK": 9, "ROT": 3, "MC":9, "MK":9 , "mC":3, "EKT": 1, "eKT": 1, "EKR": 1, "eKR": 1, "qTC": 3, "qTK": 3, "qRC":3, "qRK":3, "zT":1 , "zR":1 , "TotalStress": 9, "Pressure":1, "KineticPressure":1, "ShearStress":1, "StrainRate":9, "VolumetricStrainRate":1, "ShearStrainRate":1, "RotationalVelocity":3, "RotationalVelocityMag":1}
    if (fieldlist[document.getElementById('displayedfield').value]==0)
    {
        for (var i=0 ; i<9 ; i++) document.getElementById('component').options[0].hidden=true ;
        document.getElementById('component').value="0"
        document.getElementById('component').options[0].text="" ; 
    }
    else if (fieldlist[document.getElementById('displayedfield').value]==3)
    {
        for (var i=3 ; i<9 ; i++) document.getElementById('component').options[0].hidden=true;
        document.getElementById('component').options[0].text="x" ; document.getElementById('component').options[0].hidden=false ;
        document.getElementById('component').options[1].text="y" ; document.getElementById('component').options[1].hidden=false ;
        document.getElementById('component').options[2].text="z" ; document.getElementById('component').options[2].hidden=false ; 
        document.getElementById('component').value="0"
    }
    else if (fieldlist[document.getElementById('displayedfield').value]==9)
    {
        for (var i=0 ; i<9 ; i++) document.getElementById('component').options[0].hidden=false;
        document.getElementById('component').options[0].text="xx" ;
        document.getElementById('component').options[1].text="xy" ;
        document.getElementById('component').options[2].text="xz" ; 
        document.getElementById('component').value="0"
    }
    
    if (updated==true)
    {
        var field = document.getElementById('displayedfield').value; 
        var component = parseInt(document.getElementById('component').value); 
        var ts=parseInt(dispslider.noUiSlider.get()) ; 
        worker.postMessage(['getresult', ts, field, component]) ; 
    }
});

document.getElementById('component').addEventListener('change', async() => {
    if (updated==true)
    {
        var field = document.getElementById('displayedfield').value; 
        var component = parseInt(document.getElementById('component').value); 
        var ts=parseInt(dispslider.noUiSlider.get()) ; 
        worker.postMessage(['getresult', ts, field, component]) ; 
    }
}) ; 

document.getElementById('resetcam').addEventListener('click', async() => {
    renderer.resetCamera();
    renderer.getActiveCamera().setViewUp(0,0,1) ; 
    orientationWidget.updateMarkerOrientation()
    interactorStyle.setCenterOfRotation(renderer.getActiveCamera().getFocalPoint()) ;
    renderWindow.render();
}) ; 
// document.getElementById('camxplus').addEventListener('click', async() => {
//     renderer.getActiveCamera().setOrientationWXYZ(0,0,0,0)
//     renderer.getActiveCamera().setViewUp(0,0,1) ; 
//     renderer.resetCamera();
//     orientationWidget.updateMarkerOrientation()
//     renderWindow.render();
// }) ; 
document.getElementById('cliptype').addEventListener('change', async() => {
    var field = document.getElementById('cliptype').value; 
    
    if (field==0)
    {
        document.getElementById('cliprange').disabled=true ; 
        clipPlane1.setNormal([1,0,0]);
        clipPlane1.setOrigin([parseFloat(document.getElementById('xmin').value),0,0]);
        renderWindow.render();
    }
    else 
    {
        console.log(field) ; 
        document.getElementById('cliprange').disabled=false ; 
        document.getElementById('cliprange').min = document.getElementById(field[0]+"min").value ; 
        document.getElementById('cliprange').max = document.getElementById(field[0]+"max").value ; 
        document.getElementById('cliprange').value = (parseFloat(document.getElementById('cliprange').max)+parseFloat(document.getElementById('cliprange').min))/2
        document.getElementById('cliprangemin').innerHTML = parseFloat(document.getElementById('cliprange').min).toFixed(3) ;
        document.getElementById('cliprangemax').innerHTML = parseFloat(document.getElementById('cliprange').max).toFixed(3) ;
        if (field=="x+")
        {
            clipPlane1.setNormal([-1,0,0]);
            clipPlane1.setOrigin([parseFloat(document.getElementById('cliprange').value),0,0]);
        }
        else if (field=="x-")
        {
            clipPlane1.setNormal([+1,0,0]);
            clipPlane1.setOrigin([parseFloat(document.getElementById('cliprange').value),0,0]);
        }
        else if (field=="y+")
        {
            clipPlane1.setNormal([0,-1,0]);
            clipPlane1.setOrigin([0, parseFloat(document.getElementById('cliprange').value),0]);
        }
        else if (field=="y-")
        {
            clipPlane1.setNormal([0,1,0]);
            clipPlane1.setOrigin([0,parseFloat(document.getElementById('cliprange').value),0]);
        }
        else if (field=="z+")
        {
            clipPlane1.setNormal([0,0,-1]);
            clipPlane1.setOrigin([0,0,parseFloat(document.getElementById('cliprange').value)]);
        }
        else if (field=="z-")
        {
            clipPlane1.setNormal([0,0,1]);
            clipPlane1.setOrigin([0,0,parseFloat(document.getElementById('cliprange').value)]);
        }
        renderWindow.render();
    }
}) ; 

document.getElementById('cliprange').addEventListener('change', async() => {
    var field = document.getElementById('cliptype').value; 
    if (field=="x+" || field=="x-")
        clipPlane1.setOrigin([parseFloat(document.getElementById('cliprange').value),0,0]);
    else if (field=="y+" || field=="y-")
        clipPlane1.setOrigin([0,parseFloat(document.getElementById('cliprange').value),0]);
    else
        clipPlane1.setOrigin([0,0,parseFloat(document.getElementById('cliprange').value)]);
    renderWindow.render();
});

document.getElementById('showsphere').addEventListener('click', async() =>  {
    if (sphereinfos==false)
    {
        var ts=parseInt(dispslider.noUiSlider.get()) ; 
        worker.postMessage(['getspheres', ts]) ; 
        sphereinfos=true ; 
    }
    else
    {
        for (var n=actor2.length-1 ; n>=0 ; n--)
            renderer.removeActor(actor2[n]) ; 
        renderWindow.render();
        sphereinfos=false ;
        actor2=[] ; 
    }
}) ; 

dispslider.noUiSlider.on('change', function() {
    updated=false ; 
    if (document.getElementById('autoupdate').checked)
    {
        var ev = new MouseEvent('click', {
            view: window,
            bubbles: true,
            cancelable: true
        });
        document.getElementById('updateCG').dispatchEvent(ev);
    }
});

document.getElementById("cmin").addEventListener('change', async() => {
    var imgmin = parseFloat(document.getElementById('cmin').value) ; 
    var imgmax = parseFloat(document.getElementById('cmax').value) ; 
    lookupTable.setMappingRange(...[imgmin,imgmax]);
    lookupTable.updateRange();
    //renderer.resetCamera();
    renderWindow.render();
}) ; 

document.getElementById("cmax").addEventListener('change', async() => {
    var imgmin = parseFloat(document.getElementById('cmin').value) ; 
    var imgmax = parseFloat(document.getElementById('cmax').value) ; 
    lookupTable.setMappingRange(...[imgmin,imgmax]);
    lookupTable.updateRange();
    //renderer.resetCamera();
    renderWindow.render();
}) ; 

document.getElementById('lockcscale').addEventListener('change', async () => {
    if (document.getElementById('lockcscale').checked) 
        document.getElementById('lockcscalelbl').innerHTML="&#128274;" ; 
    else
        document.getElementById('lockcscalelbl').innerHTML="&#128275;" ; 
        
});

// Messages from the worker --------------------------------------------
worker.onmessage = function (e)
{
 if (e.data[0] == "initialised") 
 {
     if (e.data[2].length>0)
     {
        document.getElementById('xmin').value = e.data[2][0][0] ;
        document.getElementById('xmax').value = e.data[2][1][0] ; 
        document.getElementById('ymin').value = e.data[2][0][1] ; 
        document.getElementById('ymax').value = e.data[2][1][1] ; 
        document.getElementById('zmin').value = e.data[2][0][2] ; 
        document.getElementById('zmax').value = e.data[2][1][2] ;  
     }
     if (e.data[3].length>0)
     {
         document.getElementById('windowsize').value = e.data[3][1]*4 ; 
         document.getElementById('windowsize').step = Math.pow(10,Math.floor(Math.log10(document.getElementById('windowsize').value))-1) ; 
     }
     timeslider.noUiSlider.updateOptions({range: {'min': 0,'max': e.data[1]-1}}) ; 
     timeslider.noUiSlider.set([0,e.data[1]-1]) ; 
     dispslider.noUiSlider.updateOptions({range: {'min': 0,'max': e.data[1]-1}}) ; 
     dispslider.noUiSlider.set([0]) ;
     
     for (var i=0; i<lst_param.length; i++)
        document.getElementById(lst_param[i]).disabled=false ; 
     update_parameters();
     
     initialised=true; 
     document.getElementById("wait").hidden=true ; 
 }
 else if (e.data[0] == 'parametrised')
 {
    document.getElementById('updateCG').disabled=false ; 
    var select=document.getElementById('displayedfield') ; 
    while (select.children.length > 0) select.removeChild(select.children[0])
    e.data[1].forEach(element => {
        var opt = document.createElement('option');
        opt.value = element; opt.innerHTML = element;
        select.appendChild(opt);
    }) ; 
    
    if (document.getElementById("autoupdate").checked)
    {
        var ev = new MouseEvent('click', {
                view: window,
                bubbles: true,
                cancelable: true
            });
        document.getElementById('updateCG').dispatchEvent(ev);
        //renderer.resetCamera();
        renderer.getActiveCamera().setViewUp(0,0,1) ; 
        interactorStyle.setCenterOfRotation(renderer.getActiveCamera().getFocalPoint()) ;
        renderWindow.render();
    }     
}
 else if (e.data[0] == 'tsprocessed')
 {
    updated=true ; 
    var field = document.getElementById('displayedfield').value; 
    var component = parseInt(document.getElementById('component').value); 
    worker.postMessage(['getresult', e.data[1], field, component]) ;  
 }
 else if (e.data[0] == 'resultobtained')
 {
    var scalars = vtkDataArray.newInstance({
        values: e.data[1],
        numberOfComponents: 1, // number of channels (grayscale)
        dataType: VtkDataTypes.FLOAT, // values encoding
        name: 'scalars'
    });
    
    var img = vtkImageData.newInstance();
    img.setOrigin(e.data[2][0], e.data[2][1], e.data[2][2]);
    img.setSpacing(e.data[2][3], e.data[2][4], e.data[2][5]);
    img.setExtent(0, e.data[2][6]-1, 0, e.data[2][7]-1, 0, e.data[2][8]-1);
    //img.setExtent(e.data[2][0], e.data[2][0]+(e.data[2][6]-1)*e.data[2][3], e.data[2][1], e.data[2][1]+(e.data[2][7]-1)*e.data[2][4], e.data[2][2], e.data[2][2]+(e.data[2][8]-1)*e.data[2][5]); 
    img.getPointData().setScalars(scalars);
    
    console.log(document.getElementById('cliptype').value)
    if (document.getElementById('cliptype').value==0)
    {
        clipPlane1.setNormal([1,0,0]);
        clipPlane1.setOrigin([parseFloat(document.getElementById('xmin').value),0,0]);
    }    
    
    if (document.getElementById('lockcscale').checked)
    {
        var imgmin = parseFloat(document.getElementById('cmin').value) ; 
        var imgmax = parseFloat(document.getElementById('cmax').value) ; 
        lookupTable.setMappingRange(...[imgmin,imgmax]);
    }
    else
    {
        var imgmax = e.data[1].reduce(function(a, b) {return Math.max(a, b);}, 0);
        var imgmin = e.data[1].reduce(function(a, b) {return Math.min(a, b);}, 0);
        lookupTable.setMappingRange(...[imgmin,imgmax]);
        document.getElementById('cmin').value = imgmin ; 
        document.getElementById('cmax').value = imgmax ; 
    }
    lookupTable.updateRange();
    mapper.setInputData(img);
    //renderer.resetCamera();
    renderWindow.render();
 }
 else if (e.data[0] == 'sphereinfos')
 {
      console.log(e.data[1]) ; 
      const sphereSource = vtkSphereSource.newInstance({ center: [0, 0, 0], height: 1.0 });

      const mapper2 = vtkMapper.newInstance();
      mapper2.setInputConnection(sphereSource.getOutputPort());

       
      for (let n = 0; n < e.data[1][0].length; n++) 
      {
        actor2.push(vtkActor.newInstance());
        actor2[n].setMapper(mapper2);
        actor2[n].setScale(2*e.data[1][3][n], 2*e.data[1][3][n], 2*e.data[1][3][n]) ; 
        actor2[n].setPosition(e.data[1][0][n], e.data[1][1][n], e.data[1][2][n]);
        actor2[n].getProperty().setColor(Math.random(),Math.random(),Math.random()) ; 
        renderer.addActor(actor2[n]);
      }
      renderer.resetCamera();
      renderWindow.render();
 }
}









