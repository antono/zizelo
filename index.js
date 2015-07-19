var remote = require('remote');
var net = remote.require('net');

var page = {
  host: 'gohper.floodgap.com',
  port: 70,
  path: '/'
  data: [] // strings
};

function open () {
}

function append (text) {
  document.getElementById('source').innerHTML += text;
}

document.addEventListener('DOMContentLoaded', function () {
  var sock  = net.createConnection(70, 'gopher.floodgap.com');

  sock.on('data', function (data) {
    append(data.toString());
  });

  sock.write("/\n");
});
