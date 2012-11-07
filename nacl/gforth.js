function moduleDidLoad(e) {
  var module = document.getElementById('gforth');
  module.addEventListener('message', function(msg) {
    var prefix1 = 'JSPipeMount:1:';
    var prefix2 = 'JSPipeMount:2:';
    var data = '';
    if (msg.data.slice(0, prefix1.length) == prefix1) {
      data = msg.data.slice(prefix1.length);
    } else if (msg.data.slice(0, prefix2.length) == prefix2) {
      data = msg.data.slice(prefix2.length);
    }
    data = data.replace('\n', '<br>\n');
    document.getElementById('console').innerHTML += data;
  });
}

document.getElementById('gforth').addEventListener('load', moduleDidLoad, true);
