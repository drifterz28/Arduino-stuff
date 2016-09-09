var conn = new WebSocket('ws://10.0.0.114:81/', ['arduino']);
conn.onopen = function() {
  conn.send('Connect ' + new Date());
};

conn.onerror = function(error) {
  console.log('Error ', error);
};

conn.onmessage = function(e) {
  console.log('Server: ', e.data);
};

var sendRGB = debounce(function(hex) {
  conn.send(hex);
  console.log(hex)
}, 500);


function debounce(func, wait, immediate) {
  var timeout;
  return function() {
    var context = this, args = arguments;
    var later = function() {
      timeout = null;
      if(!immediate) {
        func.apply(context, args);
      }
    };
    var callNow = immediate && !timeout;
    clearTimeout(timeout);
    timeout = setTimeout(later, wait);
    if(callNow) {
      func.apply(context, args);
    }
  };
}

function parse(colorValue) {
  var value = parseInt(colorValue).toString(16)
  if(value.length < 2) {
    value = '0' + value;
  }
  return value;
}

var offState = {
  red: 0,
  green: 0,
  blue: 0,
  hex: '#ffffff'
}

var setColors = {
  Fuchsia: '#FF00FF',
  DeepPink: '#FF1493',
  DarkViolet: '#9400D3',
  DarkTurquoise: '#00CED1',
  DarkOrchid: '#9932CC',
  DarkBlue: '#00008B',
  Crimson: '#DC143C',
  Cyan: '#00FFFF',
  Chartreuse: '#7FFF00',
  Blue: '#0000FF',
  Aqua: '#00FFFF',
  HotPink: '#FF69B4',
  Purple: '#800080',
  Violet: '#EE82EE'
};


var Slider = function({color, colorValue, colorChange}) {
  return (
    <div className="slide">{color}:
      <input id={color[0]} name={color} className="slider" type="range" min="0" max="255" step="1" onChange={colorChange} value={colorValue} />
    </div>
  );
};

var SetColors = function({setColorChange, getLuma}) {
  return (
    <div className="setColors">
      {Object.keys(setColors).map(function(color, i) {
        var colorHex = setColors[color];
        var hexStyle = {
          background: colorHex,
          color: (getLuma(colorHex) < 100) ? '#fff' : '#000'
        };
        console.log(hexStyle)
        return (<button key={i} style={hexStyle} onClick={setColorChange.bind(null, colorHex)}>{color}</button>);
      })}
    </div>
  );
};

var App = React.createClass({
  getInitialState: function() {
    return offState;
  },
  turnOff: function() {
    this.setState(offState);
  },
  colorChange: function(e) {
    var state = this.state;
    var target = e.target;
    var color = {};
    color[target.name] = target.value;
    var colors = Object.assign({}, state, color);
    colors.hex = this.updateHexColor(colors);
    console.log(colors);
    this.setState(colors);
  },
  updateHexColor: function(colors) {
    var r = parse(colors.red);
    var g = parse(colors.green);
    var b = parse(colors.blue);
    return `#${r}${g}${b}`;
  },
  hexSplit: function(hexColor) {
    return [1,3,5].map(function(o) {
      return parseInt(hexColor.slice(o, o + 2), 16);
    });
  },
  setColorChange: function(hexColor) {
    var hexSplit = this.hexSplit(hexColor);
    var colors = {
      red: hexSplit[0],
      green: hexSplit[1],
      blue: hexSplit[2],
    };
    colors.hex = this.updateHexColor(colors);
    this.setState(colors);
    console.log(colors);
  },
  getLuma: function(hex) {
    var colors = this.hexSplit(hex);
    var r = colors[0];
    var g = colors[1];
    var b = colors[2];
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
  },
  render: function() {
    var hex = this.state.hex;
    var showHexColor = hex === '#ffffff' ? 'Color Display' : hex;
    var hexStyle = {
      background: hex,
      color: (this.getLuma(hex) < 100) ? '#fff' : '#000'
    };
    sendRGB(hex);
    console.log(this.getLuma(hex));
    return (
      <div className="app">
        <h1>LED Control</h1>
        <button className="off-btn" onClick={this.turnOff}>Off</button>
        <Slider color="red" colorValue={this.state.red} colorChange={this.colorChange}/>
        <Slider color="green" colorValue={this.state.green} colorChange={this.colorChange}/>
        <Slider color="blue" colorValue={this.state.blue} colorChange={this.colorChange}/>
        <div style={hexStyle} className="color">{showHexColor}</div>
        <SetColors setColorChange={this.setColorChange} getLuma={this.getLuma}/>
      </div>
    );
  }
});

ReactDOM.render(
  <App/>,
  document.querySelector('.container')
);