
export function add_plotly_download_tag(header) {
    var dimension = header.split(',').length;
    if ( !header.endsWith('\n') ) { header += '\n'; }

    document.getElementById ("download_tag").addEventListener ("click", download_data, false);
    document.getElementById ("stats").addEventListener ("mouseenter",
        () => {
            document.getElementById("download_tag").classList.remove("hidden")
            document.getElementById("download_tag").classList.add("visible")
    }, false);
    document.getElementById ("stats").addEventListener ("mouseleave",
        () => {
            document.getElementById("download_tag").classList.add("hidden")
            document.getElementById("download_tag").classList.remove("visible")
    }, false);

    function download_data() {
        let gd = document.getElementById('stats')
        let data = gd.data;
        // let header = 'Density (kg/m3),Deviatoric stress (Pa),Pressure (Pa)\n';
        let csv = '';
        if ( dimension === 2 ) {
            data.forEach( trace => {
                csv += header + trace.x.map((el, i) => [el.toFixed(4), trace.y[i].toFixed(4)].join(",")).join('\n') + '\n'
                });
        } else if ( dimension === 3 ) {
            data.forEach( trace => {
                csv += header + trace.x.map((el, i) => [el.toFixed(4), trace.y[i].toFixed(4),trace.z[i].toFixed(4)].join(",")).join('\n') + '\n'
                });
        }
        var link = document.getElementById("download_tag");
        link.setAttribute("href", encodeURI("data:text/csv;charset=utf-8,"+csv));
    }
}

let plotly_data_2d = [{
  type: 'scatter',
  mode: 'line',
  x: [],
  y: [],
  name: 'Load path 1',
  // opacity: 1,
  line: {
    width: 5,
    color: "black",
    // reversescale: false
  }
}]

export function plotly_graph(xlabel, ylabel, zlabel) {
    let layout = {
      xaxis: {
        autotick: true,
        autorange: true,
        automargin: true,
        title: xlabel //'Vertical displacement (mm)'
    },
      yaxis: {
        autotick: true,
        autorange: true,
        automargin: true,
        title: ylabel //'Vertical stress (Pa)'
    },
    legend: {
        x: 1,
        xanchor: 'right',
        y: 1
    },
    margin: {
        b: 100,
    },
    font: {
        family: 'Montserrat, Open sans',
      }
    }

    let data;

    if ( zlabel === undefined ) {
        data = plotly_data_2d;
        add_plotly_download_tag(xlabel + ',' + ylabel);
    } else {
      layout.zaxis = {
          // linecolor: 'white',
          autotick: true,
          autorange: true,
          automargin: true,
          // title: zlabel
      };
      layout.margin = {
          t: 20, //top margin
          l: 20, //left margin
          r: 20, //right margin
          b: 40 //bottom margin
      };
      layout.scene = {
              xaxis:{title: xlabel},
              yaxis:{title: ylabel},// (kg/m<sup>3</sup>)'},
              zaxis:{title: zlabel},

              camera : { eye: {x:0,y:-3,z:0} },
          }
      layout.aspectmode = 'auto';
      data = plotly_data_3d;
      add_plotly_download_tag(xlabel + ',' + ylabel + ',' + zlabel);
    }
    return { data, layout }
}

let plotly_data_3d = [{
              type: 'scatter3d',
              mode: 'lines',
              x: [],
              y: [],
              z: [],
              name: 'Load path 1',
              // opacity: 1,
              line: {
                width: 5,
                color: "black",
                // reversescale: false
              }
            }]
// export let plotly_layout_3d = {
//       // uirevision: 'true',
//       // height: 300,
//       // width: 500,
//       xaxis: {
//         // linecolor: 'white',
//         autotick: true,
//         autorange: true,
//         automargin: true,
//     },
//       yaxis: {
//         // linecolor: 'white',
//         autotick: true,
//         autorange: true,
//         automargin: true,
//     },
//       zaxis: {
//         // linecolor: 'white',
//         autotick: true,
//         autorange: true,
//         automargin: true,
//     },
//     scene: {
//         xaxis:{title: },
//         yaxis:{title: },// (kg/m<sup>3</sup>)'},
//         zaxis:{title: },
//     aspectmode: 'auto',
//         },
//     // paper_bgcolor: 'rgba(0,0,0,1)',
//     // plot_bgcolor: 'rgba(0,0,0,1)',
//     margin: {
//         t: 20, //top margin
//         l: 20, //left margin
//         r: 20, //right margin
//         b: 40 //bottom margin
//     },
//     legend: {
//         x: 1,
//         xanchor: 'right',
//         y: 1
//       }
//     }

export function plotly_two_xaxis_graph(xlabel1, xlabel2, ylabel, trace0, trace1, trace2) {
    let layout = {
          // height: 300,
          // width: 500,
          xaxis: {
            // linecolor: 'white',
            autotick: true,
            // autorange: true,
            // range: [-maxVelocity, maxVelocity],
            // range: [-1,1],
            automargin: true,
            title: xlabel1, //'Average velocity (m/s)',
            side: 'bottom'
            // title: 'Vertical displacement (mm)'
        },
          yaxis: {
            // linecolor: 'white',
            autotick: true,
            autorange: true,
            automargin: true,
            title: ylabel,//'Location (mm)',
            // color: 'black',
        },
        xaxis2: {
            autotick: true,
            autorange: true,
            automargin: true,
            title: xlabel2, //'Stress (kPa)',
            overlaying: 'x',
            side: 'top',
            rangemode: 'tozero',
            color: 'blue'
            },
        legend: {
            x: 1,
            xanchor: 'right',
            y: 1,
            // bgcolor: "rgba(0,0,0,0.01)"
            // opacity: 0.5,
        },
        margin: {
            b: 100,
        },
        font: {
            family: 'Montserrat, Open sans',
        }
    }

    let data = [{
      type: 'scatter',
      mode: 'lines',
      x: [],
      y: [],
      hoverinfo: 'skip',
      name: trace0,
      // opacity: 1,
      line: {
        width: 5,
        color: "black",
        // reversescale: false
      },
    }, {
      type: 'scatter',
      mode: 'lines',
      x: [],
      y: [],
      name: trace1,
      // opacity: 1,
      line: {
        width: 5,
        color: "blue",
        // reversescale: false
      },
      xaxis: 'x2'
    }, {
      type: 'scatter',
      mode: 'lines',
      x: [],
      y: [],
      name: trace2,
      // opacity: 1,
      line: {
        // dash: 'dash',
        dash: "8px,8px",
        width: 5,
        color: "blue",
        // reversescale: false
      },
      xaxis: 'x2'
    }]
    add_plotly_download_tag(xlabel1 + ',' + xlabel2 + ',' + ylabel);
    return { data, layout }
}

export function plotly_2d_graph(xlabel, ylabel, traces) {
  let layout = {
      showlegend : true,
        // height: 300,
        // width: 500,
        xaxis: {
          // linecolor: 'white',
          autotick: true,
          // autorange: true,
          // range: [-maxVelocity, maxVelocity],
          // range: [-1,1],
          automargin: true,
          title: xlabel, //'Average velocity (m/s)',
          side: 'bottom'
          // title: 'Vertical displacement (mm)'
      },
        yaxis: {
          // linecolor: 'white',
          autotick: true,
          autorange: true,
          automargin: true,
          title: ylabel,//'Location (mm)',
          // color: 'black',
      },
      legend: {
          x: 1,
          xanchor: 'right',
          y: 1,
          // bgcolor: "rgba(0,0,0,0.01)"
          // opacity: 0.5,
      },
      margin: {
          b: 100,
      },
      font: {
          family: 'Montserrat, Open sans',
      }
  }

  let data = [];
  // let colors = ['black',];
  traces.forEach((t,i) => {
    data.push({
      type: 'scatter',
      mode: 'lines',
      x: [],
      y: [],
      hoverinfo: 'skip',
      fill: "toself",
      fillcolor: 'black',
      name: t,
      // opacity: 1,
      line: {
        // width: 5,
        color: 'black'
        // color: colors[i],
        // reversescale: false
      },
    })
  });

  add_plotly_download_tag(xlabel + ',' + ylabel);
  return { data, layout }
}

export function plotly_2x2_graphs() {
  let xoff = 0.05;
  let yoff = 0.15;
  
  let layout = {
      grid: {
        rows: 2,
        columns: 2,
        pattern: 'independent',
        subplots:[['',''], ['','xy']],
        // roworder:'bottom to top'
      },
      polar: {
        radialaxis: {
          // title: 'Contact orientation',
          x: 0,
          xanchor: 'right',
          rangemode: 'tozero',
        },
        angularaxis: {
          rotation: 90,
          direction: "clockwise"
        },
        gridcell : 0,
        // sector: [0,180],
        domain: {
          x: [0, 0.5-xoff],
          y: [0.5+yoff, 1]
        },
      },
      polar2: {
        radialaxis: {
          // title: 'Normal force orientation',
          rangemode: 'tozero',
        },
        angularaxis: {
          rotation: 90,
          direction: "clockwise"
        },
        // sector: [0,180],
        gridcell : 1,
        domain: {
          x: [0, 0.5-xoff],
          y: [0, 0.5-yoff]
        },
      },
      polar3: {
        radialaxis: {
          // title: 'Tangential force orientation',
          rangemode: 'tozero',
        },
        angularaxis: {
          rotation: 90,
          direction: "clockwise"
        },
        // sector: [0,180],
        gridcell : 2,
        domain: {
          x: [0.5+xoff, 1],
          y: [0.5+yoff, 1]
        },
      },
      showlegend : true,
      legend: {
          x: 1,
          xanchor: 'right',
          y: 0.5,
          bgcolor: 'rgba(255, 255, 255, 0.2)'
      },
      margin: {
          b: 100,
      },
      font: {
          family: 'Montserrat, Open sans',
      },
      annotations: [{
        text: "Contact orientation",
        font: {
            size: 16,
        },
        showarrow: false,
        xanchor: 'center',
        x: (0.5-xoff)/2,
        y: 1.1,
        xref: 'paper',
        yref: 'paper',
      },
      {
        text: "Tangential force orientation",
        font: {
            size: 16,
        },
        showarrow: false,
        xanchor: 'center',
        x: 0.75+xoff/2.,
        y: 1.1,
        xref: 'paper',
        yref: 'paper',
      },
      {
        text: "Normal force orientation",
        font: {
            size: 16,
        },
        showarrow: false,
        xanchor: 'center',
        x: (0.5-xoff)/2,
        y: 0.43,
        xref: 'paper',
        yref: 'paper',
      },
      ]
  }

  let colors = ['red', 'blue', 'green', 'orange', 'black'];
  let data = [];
  let traces = ['Branch vector', 'Normal force', 'Tangential force'];
  traces.forEach((t,i) => {
    data.push({
      gridcell: i,
      showlegend: false,
      type: 'scatterpolar',
      mode: 'lines',
      r: [],
      theta: [],
      thetaunit: "radians",
      hoverinfo: 'skip',
      name: t,
      subplot: 'polar' + String(i+1),
      line: {
        width: 1,
        // color: 'black',
        color: colors[i+1],
      },
    })
    data.push({
      gridcell: i,
      showlegend: false,
      type: 'scatterpolar',
      mode: 'lines',
      r: [],
      theta: [],
      thetaunit: "radians",
      hoverinfo: 'skip',
      // name: t,
      subplot: 'polar' + String(i+1),
      line: {
        dash: 'dot',
        width: 5,
        // color: 'black',
        color: colors[i+1],
      },
    })
  });
  traces = ['Macroscopic friction','Contact anisotropy','Normal force anisotropy','Tangential force anisotropy','Rothenburg & Bathurst (1989)'];
  traces.forEach((t,i) => {
    data.push({
      gridcell: 3,
      type: 'scatter',
      mode: 'lines',
      x: [],
      y: [],
      hoverinfo: 'skip',
      name: t,
      line: {
        width: 1,
        color: colors[i]
      },
    })
  });

  data[6].line.width = 5;
  data[10].line.width = 5;

  let xlabels = 'TEST'
  let ylabels = 'TEST'
  add_plotly_download_tag(xlabels + ',' + ylabels);

  var config = {responsive: true}

  return { data, layout, config }
}