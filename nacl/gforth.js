var forth;
var accept_buffer = '';
var event_buffer = '';
var last_output = '';
var escaping = 0;

function startEvents() {
  document.onkeypress = function(evt) {
    var ch = evt.which;
    if (ch == undefined) ch = evt.keyCode;
    event_buffer += String.fromCharCode(ch);
    processKeys();
    return false;
  };
  document.onkeydown = function(evt) {
    event_buffer += String.fromCharCode(evt.keyCode + 256);
    processKeys();
    if (evt.keyCode == 8) return false;
    return true;
  };
  document.onkeyup = function(evt) {
    event_buffer += String.fromCharCode(evt.keyCode + 512);
    processKeys();
    if (evt.keyCode == 8) return false;
    return true;
  };
  document.onpaste = function(evt) {
    var data = evt.clipboardData.getData('text/plain');
    data = data.replace(/\u2003/g, ' ');
    data = data.replace(/\r/g, '');
    data = data.replace(/\n/g, '\r');
    event_buffer += data;
    processKeys();
    return true;
  };
}

function moduleDidLoad(e) {
  startEvents();
  document.getElementById('out').innerHTML = '';
  forth = document.getElementById('gforth');
  forth.addEventListener('message', function(msg) {
    var prefix1 = 'JSPipeMount:1:';
    var prefix2 = 'JSPipeMount:2:';
    var data = '';
    if (msg.data.slice(0, prefix1.length) == prefix1) {
      data = msg.data.slice(prefix1.length);
    } else if (msg.data.slice(0, prefix2.length) == prefix2) {
      data = msg.data.slice(prefix2.length);
    }
    if (escaping > 0) {
      --escaping;
      return;
    }
    for (var i = 0; i < data.length; i++) {
      var ch = data.substr(i, 1);
      if (ch == '\x1b') {
        document.getElementById('out').innerHTML = '';
        escaping = 8;
        return;
      }
    }
    appendOut(htmlEscape(data));
  });
}

function htmlEscape(str) {
  return str.replace(
      /&/g, '&amp;').replace(
      />/g, '&gt;').replace(
      /</g, '&lt;').replace(
      /"/g, '&quot;');
}

function processKeys() {
  for (var i = 0; i < event_buffer.length; i++) {
    var ch = event_buffer.substr(i, 1);
    var code = event_buffer.charCodeAt(i);
    if (code == 8 || code == 8 + 256) {
      if (accept_buffer.length) {
        accept_buffer = accept_buffer.substr(
            0, accept_buffer.length - 1);
      }
    } else if (code == 13) {
      var line = accept_buffer;
      accept_buffer = '';
      appendOut(htmlEscape(line) + '\n');
      event_buffer = event_buffer.substr(i + 1);
      refreshPrompt();
      forth.postMessage('JSPipeMount:0:' + line + '\n');
      return;
    } else if (code >= 32 && code <= 126) {
      accept_buffer += ch;
    }
  }
  event_buffer = '';
  refreshPrompt();
}

function appendOut(text) {
  var span = document.createElement('span');
  span.innerHTML = text;
  var out = document.getElementById('out');
  out.appendChild(span);
}

function refreshPrompt() {
  var prompt = document.getElementById('prompt');
  prompt.innerHTML = htmlEscape(accept_buffer);
  window.scrollTo(0, document.body.scrollHeight);
}

document.getElementById('gforth').addEventListener('load', moduleDidLoad, true);
