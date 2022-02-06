// Load the rendering pieces we want to use (for both WebGL and WebGPU)
import '@kitware/vtk.js/Rendering/Profiles/Volume';

import vtkColorTransferFunction from '@kitware/vtk.js/Rendering/Core/ColorTransferFunction';
import vtkDataArray from '@kitware/vtk.js/Common/Core/DataArray';
import vtkFullScreenRenderWindow from '@kitware/vtk.js/Rendering/Misc/FullScreenRenderWindow';
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
import vtkAxesActor from '@kitware/vtk.js/Rendering/Core/AxesActor';


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

var dispts = 0 ;

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
const clipPlane2 = vtkPlane.newInstance();
let clipPlane1Position = 0;
let clipPlane2Position = 0;
let clipPlane1RotationAngle = 0;
let clipPlane2RotationAngle = 0;
const clipPlane1Normal = [-1, 1, 0];
const clipPlane2Normal = [0, 0, 1];
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
actor.getProperty().setInterpolationTypeToLinear();
actor.getProperty().setUseGradientOpacity(0, false);
actor.getProperty().setShade(false);
actor.getProperty().setAmbient(1);
actor.getProperty().setDiffuse(0.7);
actor.getProperty().setSpecular(0.3);
actor.getProperty().setSpecularPower(8.0);

    var width = 3, height = 3, depth = 3;
    var size = width * height * depth;

    var values = [];
    for (var i = 0; i < size; i++) {
        values[i] = i/26*255;
    }

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

// clipPlane1.setNormal([1,0,0]);
// clipPlane1.setOrigin([25,25,25]);
// mapper.addClippingPlane(clipPlane1);

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
renderWindow.getInteractor().setInteractorStyle(interactorStyle);

const axes = vtkAxesActor.newInstance({ pickable: true });
const orientationWidget = vtkOrientationMarkerWidget.newInstance({
  actor: axes,
  interactor: interactor,
});
// orientationWidget.setViewportCorner(
//   vtkOrientationMarkerWidget.Corners.BOTTOM_LEFT
// );
orientationWidget.setEnabled(true) ; 
orientationWidget.setMinPixelSize(100);
orientationWidget.setMaxPixelSize(300);
// renderer
//   .getActiveCamera()
//   .onModified(orientationWidget.updateMarkerOrientation);

renderWindow.render();



renderer.resetCamera();
renderer.getActiveCamera().elevation(70);
renderWindow.render();

/*document.querySelector('.plane1Position').addEventListener('input', (e) => {
  clipPlane1Position = Number(e.target.value);
  const clipPlane1Origin = [
    clipPlane1Position * clipPlane1Normal[0],
    clipPlane1Position * clipPlane1Normal[1],
    clipPlane1Position * clipPlane1Normal[2],
  ];import controlPanel from './controller.html';
  clipPlane1.setOrigin(clipPlane1Origin);
  renderWindow.render();
});

document.querySelector('.plane1Rotation').addEventListener('input', (e) => {
  const changedDegree = Number(e.target.value) - clipPlane1RotationAngle;
  clipPlane1RotationAngle = Number(e.target.value);
  vtkMatrixBuilder
    .buildFromDegree()
    .rotate(changedDegree, rotationNormal)
    .apply(clipPlane1Normal);
  clipPlane1.setNormal(clipPlane1Normal);
  renderWindow.render();
});

document.querySelector('.plane2Position').addEventListener('input', (e) => {
  clipPlane2Position = Number(e.target.value);
  const clipPlane2Origin = [
    clipPlane2Position * clipPlane2Normal[0],
    clipPlane2Position * clipPlane2Normal[1],
    clipPlane2Position * clipPlane2Normal[2],
  ];
  clipPlane2.setOrigin(clipPlane2Origin);
  renderWindow.render();
});

document.querySelector('.plane2Rotation').addEventListener('input', (e) => {
  const changedDegree = Number(e.target.value) - clipPlane2RotationAngle;
  clipPlane2RotationAngle = Number(e.target.value);
  vtkMatrixBuilder
    .buildFromDegree()
    .rotate(changedDegree, rotationNormal)
    .apply(clipPlane2Normal);
  clipPlane2.setNormal(clipPlane2Normal);
  renderWindow.render();
});*/

// -----------------------------------------------------------
// Make some variables global so that you can inspect and
// modify objects in your browser's developer console:
// -----------------------------------------------------------

// global.source = reader;
// global.mapper = mapper;
// global.actor = actor;
// global.renderer = renderer;
// global.renderWindow = renderWindow;
// global.clipPlane1 = clipPlane1;
// global.clipPlane2 = clipPlane2;

/*var fileHandle;

var worker = new Worker("worker.js");

document.querySelector('.butfile').addEventListener('click', async () => {
    
    var file = document.getElementById('filename').files[0];
    worker.postMessage([ file ]);
});


let CGLiggghts ; 
let CGAPI ; 
async function CGLigghts_em ()
{
CGLiggghts = await CoarseGraining_Liggghts();
global.CGLiggghts = CGLiggghts;
console.log(CGLiggghts) ; 
CGAPI = await new CGLiggghts.CGAPI() ;
CGAPI.parse('{"particles": "dump.testSegre006","forces": "dump.forceSegre006","fields" : ["RHO", "VAVG", "TC", "TK"],"savefile": "Segre006", "saveformat": ["vtk", "mat", "numpy"],"periodicity": [true, true, false],"boundaries": [[-0.0075, -0.0075, 0],[0.0075, 0.0075, 0.015]], "boxes": [1,1,20],"skip": 150,"max time": 350,"time average": true,"window size": 0.0015,"window": "Lucy3DFancyInt","mapping": {  "id1": "c_cout[1]",  "id2": "c_cout[2]",  "per": "c_cout[3]", "fx": "c_cout[4]",  "fy": "c_cout[5]", "fz": "c_cout[6]"}}') ;   
CGAPI.initialize() ; 
CGAPI.skip_ts() ; 
CGAPI.read_ts() ;
CGAPI.process_full() ; 
    
}

CGLigghts_em() ;*/


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
var timeslider=document.getElementById('slider') ; 
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

//--------------------- File handling -------------------------------------
var naddedfiles=0  ; 
document.querySelector('.addfile').addEventListener('click', async () => {
    naddedfiles ++ ;
    document.getElementById("remfile").hidden=false ; 
    if (naddedfiles>2) {naddedfiles=2 ; return ; }
    var idstr = "file"+naddedfiles ; 
    document.getElementById(idstr).hidden=false ; 
});
document.getElementById('remfile').addEventListener('click', async () => {
    if (naddedfiles<0) {naddedfiles=0 ; document.getElementById("remfile").hidden=true ;  return ;}
    var idstr = "file"+naddedfiles ; 
    document.getElementById(idstr).hidden=true ; 
    naddedfiles -- ;
    if (naddedfiles==0) {document.getElementById("remfile").hidden=true ; }
});
//-----
var ncolumnmapping=0 ; 
document.getElementById('addcolumn').addEventListener('click', async () => {
    ncolumnmapping++ ; 
    var fragment = document.createDocumentFragment();
    var l0 = document.createElement("br")
    fragment.appendChild(l0) ; l0.id="columnspacing"+ncolumnmapping ;
    var l1 = document.createElement("input")
    l1.type="text" ; l1.id="columnname"+ncolumnmapping ; l1.placeholder="Column name"
    fragment.appendChild(l1) ; 
    var l01 = document.createElement("span")
    l01.textContent="=>" /*"&rarr;"*/ ; l01.id="columnseparator"+ncolumnmapping ; 
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
    l4.textContent="X"/*&#2717;"*/ ; l4.id="columnbutton"+ncolumnmapping ;
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
});
//----------
document.getElementById('RHO').addEventListener('change', async () => {
    if (document.getElementById('RHO').checked==false)
        alert('The density is needed for most field calculation. It is strongly recommended to keep it checked') ; 
});
function setstresseschecked() {
        document.getElementById('TC').checked=true ; 
        document.getElementById('TK').checked=true ; 
}
function setvelocitychecked() { document.getElementById('VAVG').checked=true ; }
document.getElementById('TotalStress').addEventListener('change', async () => { if (document.getElementById('TotalStress').checked==true) setstresseschecked() ; });
document.getElementById('Pressure').addEventListener('change', async () => { if (document.getElementById('Pressure').checked==true) setstresseschecked() ; });
document.getElementById('KineticPressure').addEventListener('change', async () => {if (document.getElementById('KineticPressure').checked==true) setstresseschecked() ; });
document.getElementById('ShearStress').addEventListener('change', async () => { if (document.getElementById('ShearStress').checked==true) setstresseschecked() ; });

document.getElementById('VolumetricStrainRate').addEventListener('change', async () => { if (document.getElementById('VolumetricStrainRate').checked==true) setvelocitychecked() ; });
document.getElementById('ShearStrainRate').addEventListener('change', async () => { if (document.getElementById('ShearStrainRate').checked==true) setvelocitychecked() ; });
document.getElementById('RotationalVelocity').addEventListener('change', async () => { if (document.getElementById('RotationalVelocity').checked==true) setvelocitychecked() ; });
document.getElementById('RotationalVelocityMag').addEventListener('change', async () => { if (document.getElementById('RotationalVelocityMag').checked==true) setvelocitychecked() ; });
//---------------------
function generateallparameters () {
    var data = {}
    
    var files = []
    for (var fic = 1 ; fic <=naddedfiles ; fic++)
    {
        files[fic-1] = {} ; 
        var filelist = document.getElementById("filename"+fic) ; 
        files[fic-1]["number"] = filelist.files.length ;
        var tmp = filelist.files[0].name ;
        if (filelist.files.length>1) tmp.replace(/[0-9]+/i,"%d") ; 
        files[fic-1]["filename"] = tmp ; 
        
        files[fic-1]["filename"] = document.getElementById("filename"+fic).value ; 
        var filetype = document.getElementById("filetype"+fic).value ; 
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
        
        if (ncolumnmapping>0 && fic==2)
        {
            files[fic-1]["mapping"]={} ; 
            for (var column=1 ; column <= ncolumnmapping ; column++) 
            {
                try {
                    files[fic-1]["mapping"][document.getElementById("columnselect"+column).value] = document.getElementById("columnname"+column).value ; 
                } catch (e) {continue ;}
            }
        }
    }
    data["file"]=files ; 
    
    data["boxes"] = [parseInt(document.getElementById("xgrid").value), parseInt(document.getElementById("ygrid").value), parseInt(document.getElementById("zgrid").value)] ;
    data["boundaries"] = [[parseFloat(document.getElementById("xmin").value), parseFloat(document.getElementById("ymin").value), parseFloat(document.getElementById("zmin").value)], 
                          [parseFloat(document.getElementById("xmax").value), parseFloat(document.getElementById("ymax").value), parseFloat(document.getElementById("zmax").value)]] ;
    data["periodicity"] = [document.getElementById("xper").checked, document.getElementById("yper").checked, document.getElementById("zper").checked] ;
                          
    data["window size"] = parseFloat(document.getElementById("windowsize").value) ; 
    data["window"] =document.getElementById("window").value ;
    
    var times=timeslider.noUiSlider.get() ; 
    data["skip"]=parseInt(times[0]) ; 
    data["max time"]=parseInt(times[1])-parseInt(times[0]) ;
    
    data["time average"]=document.getElementById("timeaverage").value ; 
    
    var fieldlst= ["RHO", "I", "VAVG",  "TC", "TK", "ROT", "MC", "MK" , "mC", "EKT", "eKT", "EKR", "eKR", "qTC", "qTK", "qRC", "qRK", "zT" , "zR", "TotalStress", "Pressure", "KineticPressure", "ShearStress", "StrainRate", "VolumetricStrainRate", "ShearStrainRate", "RotationalVelocity", "RotationalVelocityMag"] ;    
    data["fields"]=[]
    fieldlst.forEach(name => {if (document.getElementById(name).checked==true) data["fields"].push(name);}) ; 
    
    slider.noUiSlider.get();
    
    var select = document.getElementById('saveformat');
    data["saveformat"] = [...document.getElementById('saveformat').selectedOptions].map(option => option.value);
    data["save"] = document.getElementById('savefile').value; 
    
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
    
        var fieldlst= ["RHO", "I", "VAVG",  "TC", "TK", "ROT", "MC", "MK" , "mC", "EKT", "eKT", "EKR", "eKR", "qTC", "qTK", "qRC", "qRK", "zT" , "zR", "TotalStress", "Pressure", "KineticPressure", "ShearStress", "StrainRate", "VolumetricStrainRate", "ShearStrainRate", "RotationalVelocity", "RotationalVelocityMag"] ;    
        fieldlst.forEach(name => document.getElementById(name).checked = false) ; 
        data["fields"].forEach(name => document.getElementById(name).checked = true)
        /* TODO
        * var select = document.getElementById('saveformat');
        data["saveformat"] = [...document.getElementById('saveformat').selectedOptions].map(option => option.value);
        data["save"] = document.getElementById('savefile').value; */

    }}) ; 
//------------------
var worker = new Worker("worker.js");
var initialised=false ; 
var updated=false
document.getElementById('updatefile').addEventListener('click', async () => {
    var data=generateallparameters() ;
    if (naddedfiles == 1) 
    {
        worker.postMessage([ 'initialise', data, document.getElementById("filename1").files]);
    }
    else if (naddedfiles==2)
        worker.postMessage([ 'initialise', data, document.getElementById("filename1").files[0], document.getElementById("filename2").files[0]]);
});

document.getElementById('applyparam').addEventListener('click', async () => {
    var data=generateallparameters() ;
    if (initialised == false)
        alert("You need first to update the file(s) in the first tab.") ; 
    else
        worker.postMessage(['setparameters', data]) ;
    updated=false ; 
});

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
     timeslider.noUiSlider.updateOptions({range: {'min': 0,'max': e.data[1]-1}}) ; 
     timeslider.noUiSlider.set([0,e.data[1]-1]) ;
     dispslider.noUiSlider.updateOptions({range: {'min': 0,'max': e.data[1]-1}}) ; 
     dispslider.noUiSlider.set([0]) ;
     initialised=true; 
 }
 else if (e.data[0] == 'parametrised')
 {
    document.getElementById('updateCG').disabled=false ; 
    var select=document.getElementById('displayedfield') ; 
    e.data[1].forEach(element => {
        var opt = document.createElement('option');
        opt.value = element; opt.innerHTML = element;
        select.appendChild(opt);
    }) ; 
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
     console.log(e.data[1]) ; 
     console.log(e.data[2]) ; 

    var test=[] ; 
    for (i=0 ; i<27 ; i++) test[i]=(26-i)/27*255 ;
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
    img.getPointData().setScalars(scalars);
    
    var imgmax = e.data[1].reduce(function(a, b) {return Math.max(a, b);}, 0);
    var imgmin = e.data[1].reduce(function(a, b) {return Math.min(a, b);}, 0);
    lookupTable.setMappingRange(...[imgmin,imgmax]);
    lookupTable.updateRange();
    mapper.setInputData(img);
    renderer.resetCamera();
    renderWindow.render();
 }
    
    
}
