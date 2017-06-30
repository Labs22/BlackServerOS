/* Crypton Client, Copyright 2013 SpiderOak, Inc.
 *
 * This file is part of Crypton Client.
 *
 * Crypton Client is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Client is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Client.  If not, see <http://www.gnu.org/licenses/>.
*/

$(document).ready(function () {
  $('#login input').first().focus();
  $('#login input[type=submit]').click(function (e) {
    e.preventDefault();
    var action = e.target.name;
    var data = $('#login form').serializeArray();
    var username = data[0].value;
    var password = data[1].value;
    actions[action](username, password);
  });
});

var actions = {};

actions.login = function (username, password) {
  var $form = $('#login form');
  var $status = $('#login #status');

  $form.hide();
  $status.show();
  $status.text('Logging in...');

  crypton.authorize(username, password, function (err, session) {
    if (err) {
      $form.show();
      $status.text(err);
      $('#login input').first().focus();
      return;
    }

    window.session = session;
    $('#login').hide();
    $('body').removeClass('login');
    init();
  });
};

actions.register = function (username, password) {
  var $form = $('#login form');
  var $status = $('#login #status');
  var $progressBar = $('#login .progressBar');
  var $progress = $progressBar.find('.progress');

  $form.hide();
  $status.show();
  $progressBar.show();
  $status.text('Generating encryption keys...');

  var messages = [
    'Generating encryption keys...',
    'Doing a bunch of math...',
    'Making your diary really, really safe...'
  ];

  var steps = 0;
  crypton.generateAccount(username, password, function done (err, account) {
    if (err) {
      $form.show();
      $status.text(err);
      $progressBar.hide();
      $('#login input').first().focus();
    } else {
      actions.login(username, password);
    }
  });
};

function init () {
  $('#app').show();
  setTimeout(function () {
    $('#header').removeClass('hiding');
  }, 200);
  loadDiary();

  $('#create').click(createEntry);
  $('#display').click(displayEntries);
  $('#save').click(saveEntry);
  $('#delete').click(deleteEntry);
}

function setStatus (message) {
  $('#header span').text(message);
}

function loadDiary () {
  setStatus('Loading diary...');

  session.load('diary', function (err, container) {
    if (err) {
      session.create('diary', function (err, container) {
        window.diary = container;
        loadEntries();
      });
      return;
    }

    window.diary = container;
    loadEntries();
  });
}

function loadEntries () {
  setStatus('Loading entries...');
  diary.add('entries', function () {
    diary.get('entries', function (err, entries) {
      window.entries = entries;
      setStatus('');
      displayEntries();
    });
  });
}

function displayEntries () {
  var keys = Object.keys(entries);

  if (!keys.length) {
    setStatus('No saved entries');
    createEntry();
    return;
  }

  if (!checkEntry()) {
    return;
  }

  setStatus('Rendering entries...');
  clearInterval(dateRenderLoop);
  $('#pad').removeClass('active');
  entry = undefined;
  var $list = $('#entryList');
  $list.empty();
  $list.addClass('active');

  // sort the entries by timestamp descending
  keys.sort();
  keys.reverse();

  var $ul = $('<ul />');
  for (var i in keys) {
    var $li = $('<li />');
    var time = entries[keys[i]].time;
    $li.text(prettyDate(time));
    $li.data('time', time);
    $li.click(clickEntry);
    $ul.append($li);
  }

  $list.append($ul);
  setStatus('');
}

function showPad () {
  var $list = $('#entryList');
  var $pad = $('#pad');
  $list.removeClass('active');
  $pad.addClass('active');
  $pad.find('textarea').focus().val(entry.body);
}

function clickEntry () {
  var time = $(this).data('time');
  getEntry(time);
}

function createEntry () {
  window.entry = {};
  entry.time = +new Date();
  entry.body = '';
  
  showPad();
  renderDate();
}

var dateRenderLoop;
function renderDate () {
  $('#date').text('Dated ' + prettyDate(entry.time));
  dateRenderLoop = setInterval(function () {
    $('#date').text('Dated ' + prettyDate(entry.time));
  }, 1000);
}

function checkEntry () {
  if (typeof entry != 'undefined') {
    var text = $('textarea').val();

    // if it's not saved
    if (!entries[entry.time]) {
      // if it's not  blank
      if (text.length) {
        return confirmCheck();
      }
    } else {
      // it is saved
      // if it's changed
      if (text != entries[entry.time].body) {
        return confirmCheck();
      }
    }
  }

  return true;

  function confirmCheck () {
    return confirm('You have unsaved changes. Are you sure you want to leave this page?');
  }
}

function getEntry (time) {
  if (!checkEntry()) {
    return;
  }

  window.entry = entries[time];
  showPad();
  renderDate();
}

function saveEntry () {
  entry.body = $('textarea').val();
  entries[entry.time] = entry;
  saveDiary();
}

function deleteEntry () {
  var sure = confirm('Are you sure you want to delete this entry?');
  if (!sure) return;
  delete entries[entry.time];
  saveDiary();
  displayEntries();
}

function saveDiary () {
  setStatus('Saving diary...');
  diary.save(function (err) {
    setStatus(err || 'Diary saved');
  });
}

function prettyDate (time) {
  var d = new Date();
  var date = new Date(time);
  var diff = ((d.getTime() - date.getTime()) / 1000);
  var day_diff = Math.floor(diff / 86400);
  if (isNaN(day_diff) || day_diff < 0 || day_diff >= 31) {
    return;
  }

  return day_diff === 0 && (
        diff < 60 && "just now" ||
        diff < 120 && "1 minute ago" ||
        diff < 3600 && Math.floor( diff / 60 ) + " minutes ago" ||
        diff < 7200 && "1 hour ago" ||
        diff < 86400 && Math.floor( diff / 3600 ) + " hours ago") ||
    day_diff == 1 && "Yesterday" ||
    day_diff < 7 && day_diff + " days ago" ||
    day_diff < 31 && Math.ceil( day_diff / 7 ) + " week ago";
}
