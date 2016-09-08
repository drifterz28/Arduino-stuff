var conn = new WebSocket('ws://10.0.0.114:81/', ['arduino']);
var slider = document.querySelectorAll('.slider');
var color = document.querySelector('.color');
var offBtn = document.querySelector('.off-btn');
conn.onopen = function() {
  conn.send('Connect ' + new Date());
};

conn.onerror = function(error) {
  console.log('Error ', error);
};

conn.onmessage = function(e) {
  console.log('Server: ', e.data);
};
function parse(elm) {
  return parseInt(elm.value).toString(16);
}
function debounce(func, wait, immediate) {
  var timeout;
  return function() {
    var context = this, args = arguments;
    var later = function() {
      timeout = null;
      if (!immediate) func.apply(context, args);
    };
    var callNow = immediate && !timeout;
    clearTimeout(timeout);
    timeout = setTimeout(later, wait);
    if (callNow) func.apply(context, args);
  };
}
function off() {
  for (var i = 0; i < slider.length; i++) {
    slider[i].value = 0;
  }
  sendRGB();
}
var sendRGB = debounce(function() {
  var r = parse(slider[0]);
  var g = parse(slider[1]);
  var b = parse(slider[2]);
  if(r.length < 2) {
    r = '0' + r;
  }
  if(g.length < 2) {
    g = '0' + g;
  }
  if(b.length < 2) {
    b = '0' + b;
  }
  var rgb = '#' + r + g + b;
  conn.send(rgb);
  color.style.backgroundColor = rgb;
  color.textContent = rgb;
  console.log(rgb)
}, 200);

for (var i = 0; i < slider.length; i++) {
  slider[i].addEventListener('change', sendRGB);
}
offBtn.addEventListener('click', off);
