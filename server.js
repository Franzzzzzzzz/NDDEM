var express = require('express');
var cors = require('cors');
//const { exec } = require('child_process');
//const glob = require('glob');
//var responseTime = require('response-time');

var server = express();
//server.use(responseTime());
server.use(cors());


const logRequestStart = (req, res, next) => {
    console.info(`${req.method} ${req.originalUrl}`)
    next()
}
// server.use(logRequestStart);

server.use('/', express.static(__dirname + '/')); // serve data files

// server.post('/in', function(req, res) { // serve infiles
//     var fname = req.query.fname;
//     glob(fname + "in.*", {}, (err, infile)=>{
//         console.log("Sending " + infile[0])
//         res.sendFile( __dirname +  '/' + infile[0] );
//     });
// });

server.listen(54321); // run server
