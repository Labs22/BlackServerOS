/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Cam Pedersen <cam@campedersen.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

// IMPORTANT NOTE:
// This is a modified copy of the code that can be found at:
//
//  https://github.com/ecto/isomer
//
// This version however is unique and distinct from that codebase..

(function () {

self.isomerize = function (obj, dependencies) {
  // if we don't have access to web workers,
  // just run everything in the main thread
  if (!window.Worker) {
    return;
  }

  var CircularJSON = window.CircularJSON;

  var code = '';

  if (typeof dependencies == 'string') {
    code += 'window = document = self; self.worker = true;\n';
    code += 'document.documentElement = { style: [] };\n'; // fix for socket.io
    code += 'importScripts(\'' + dependencies + '\');\n';
  } else {
    for (var i in dependencies) {
      var name = dependencies[i];
      var dep = window[name];
      code += toSource(dep, name);
    }
  }

  code += toSource(obj, 'original');
  code += '(' + isomerExternal.toString() + ')();';

  var blob = new Blob([ code ], { type: 'application/javascript' });
  var worker = new Worker(URL.createObjectURL(blob));
  var listeners = {};

  worker.onmessage = function (e) {
    var data = CircularJSON.parse(e.data);

    if (!listeners[data.time]) {
      return;
    }

    listeners[data.time].apply(listeners[data.time], data.args);
  };

  for (var i in obj) {
    if (typeof obj[i] == 'function') {
      obj[i] = overrideLocalMethod(i);
    }
  }

  function overrideLocalMethod (methodName) {
    return function isomerProxy () {
      var args = [].slice.call(arguments);
      var callback = args.pop();
      var now = +new Date();
      listeners[now] = callback;

      worker.postMessage(CircularJSON.stringify({
        name: methodName,
        time: now,
        args: args
      }));
    }
  }
}

function isomerExternal () {
  onmessage = function (e) {
    var data = CircularJSON.parse(e.data);
    var args = data.args;

    args.push(function () {
      var args = [].slice.call(arguments);
      postMessage(CircularJSON.stringify({
        time: data.time,
        args: args
      }));
    });

    original[data.name].apply(original, args);
  }
}

function toSource (obj, name) {
  var code = '';

  if (name) {
    code += 'var ' + name + ' = ';
  }

  if (typeof obj == 'function') {
    code += obj.toString();
  } else {
    code += CircularJSON.stringify(obj);
  }

  code += ';\n';

  for (var i in obj) {
    if (typeof obj[i] != 'function') {
      continue;
    }

    if (name) {
      code += name + '.' + i + ' = ';
    }

    code += obj[i].toString() + ';\n';
  }

  for (var i in obj.prototype) {
    if (name) {
      code += name + '.prototype.' + i + ' = ';
    }

    if (typeof obj.prototype[i] == 'function') {
      code += obj.prototype[i].toString() + ';\n';
    } else if (typeof obj.prototype[i] == 'object') {
      code += CircularJSON.stringify(obj.prototype[i]) + ';\n';
    }
  }

  return code;
}


})();
