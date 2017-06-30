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
  crypton.host = window.location.host;

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
  loadDatabase();

  $('#display').click(displayConversations);
  $('#create').click(function () {
    createConversation();
  });

  session.on('message', function (message) {
    if (message.headers.app == 'chat' && message.headers.type == 'message') {
      receiveMessage(message.payload);
      session.inbox.delete(message.messageId, function (err) {
        if (err) {
          console.log(err);
        }
      });
    }
  });
}

function setStatus (message) {
  $('#header span').text(message);
}

function loadDatabase () {
  setStatus('Loading database...');

  session.load('chat', function (err, container) {
    if (err) {
      session.create('chat', function (err, container) {
        window.db = container;
        loadConversations();
      });
      return;
    }

    window.db = container;
    loadConversations();
  });
}

function loadConversations () {
  setStatus('Load conversations...');
  db.add('conversations', function () {
    db.get('conversations', function (err, conversations) {
      window.conversations = conversations;
      setStatus('');
      displayConversations();
    });
  });
}

function displayConversations () {
  var users = Object.keys(conversations);

  if (!users.length) {
    setStatus('No previous conversations');
    setTimeout(function () {
      createConversation();
    }, 200);
    return;
  }

  setStatus('Rendering conversations...');
  clearInterval(dateRenderLoop);
  $('#conversation').removeClass('active');
  conversation = undefined;
  var $list = $('#conversationList');
  $list.empty();
  $list.addClass('active');

  // sort the conversations by timestamp descending
  users.sort();
  users.reverse();

  var $ul = $('<ul />');
  for (var i in users) {
    var $li = $('<li />');
    var username = users[i];
    var time = conversations[users[i]];

    $li.data('time', time);
    $li.data('username', username);

    $li.append('<strong>' + username + '</strong><br />');
    $li.append(prettyDate(time));

    $li.click(function () {
      createConversation($(this).data('username'));
    });

    $ul.append($li);
  }

  $list.append($ul);
  setStatus('');
}

function createConversation (username) {
  if (!username) {
    username = prompt('Enter a username to start a conversation with:');
  }

  if (username) {
    session.getPeer(username, function (err, peer) {
      if (err) {
        alert(err);
        return;
      }

      window.currentPeer = peer;
      conversations[peer.username] = +new Date();
      db.save(function (err) {
        renderConversation(peer);
      });
    });
  }
}

function renderConversation (peer) {
  setStatus('Loading conversation...');

  db.add('conversation:' + peer.username, function () {
    db.get('conversation:' + peer.username, function (err, conversation) {
      window.conversation = conversation;
      clearMessages();

      if (!conversation.messages) {
        conversation.messages = [];
      } else {
        for (var i in conversation.messages) {
          addMessage(conversation.messages[i]);
        }
      }

      setStatus('Conversation with ' + peer.username);
      $('#conversationList').removeClass('active');
      $('#conversation').addClass('active');
      $('#conversationInput').focus().keydown(function (e) {
        if (e.which != 13) {
          return;
        }

        var text = $(this).val();
        $(this).val('');

        if (text == '') {
          return;
        }

        var message = {
          from: session.account.username,
          to: peer.username,
          body: text
        };

        sendMessage(message);
      });
    });
  });
}

function clearMessages () {
  $('#messages').empty();
}

function addMessage (message) {
  var $messages = $('#messages');
  var $message = $('<li />');
  $message.html('<strong>' + message.from + ':</strong> ' + message.body);
  $messages.append($message);
  $messages.scrollTop($messages[0].scrollHeight);
}

function sendMessage (message) {
  conversations[message.to] = +new Date();
  conversation.messages.push(message);

  db.save(function (err) {
    currentPeer.sendMessage({
      app: 'chat',
      type: 'message'
    }, message, function (err, mid) {
      if (err) {
        alert(err);
        return;
      }

      addMessage(message);
    });
  });
}

function receiveMessage (message) {
  // get appropriate conversation
  conversations[message.from] = +new Date();
  db.add('conversation:' + message.from, function () {
    db.get('conversation:' + message.from, function (err, temporaryConversation) {
      if (!temporaryConversation.messages) {
        temporaryConversation.messages = [];
      }

      temporaryConversation.messages.push(message);

      db.save(function (err) {
        createConversation(message.from);
      });
    });
  });

}

function processInbox () {

}

var dateRenderLoop;
function renderDate () {
  $('.time').text(prettyDate(entry.time));
  dateRenderLoop = setInterval(function () {
    $('.time').text(prettyDate(entry.time));
  }, 1000);
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
