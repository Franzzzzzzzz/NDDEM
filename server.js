var express = require('express');
var cors = require('cors');
const { exec } = require('child_process');
const glob = require('glob');

var server = express();
server.use(cors());

const logRequestStart = (req, res, next) => {
    console.info(`${req.method} ${req.originalUrl}`)
    next()
}
// server.use(logRequestStart)

server.use('/', express.static(__dirname + '/')); // serve data files

server.post('/in', function(req, res) { // serve infiles
    var fname = req.query.fname;
    glob(fname + "in.*", {}, (err, infile)=>{
        console.log("Sending " + infile[0])
        res.sendFile( __dirname +  '/' + infile[0] );
    });
});

server.post('/make_textures', function(req, res) { // generate textures
    var N = parseInt(req.query.N);
    var t = req.query.t;
    var quality = req.query.quality;
    var arr = JSON.parse(req.query.arr);
    var fname = req.query.fname;
    var text_arg = quality + " " + N + " " + arr.join(' ') + " " + t + " " + fname;
    console.log(text_arg);
    exec('./CppCode/Texturing/Texturing ' + text_arg, (err, stdout, stderr) => { res.send(stdout); } );
});

server.listen(8000); // run server
