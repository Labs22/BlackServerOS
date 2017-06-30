/*
conversations
  bob:
    last_message
      timestamp
      text
 textA   mine_name
    theirs_hmac

conversation_bob
  messages (array)
    { timestamp, action, text }

----

let's say alice wants to chat with bob
1) she creates a container for her messages and actions
2) she shares the container with bob
3) she adds "conversation with bob" to her conversation list

4) bob's client parses the message
5) bob loads the container and adds its hmac to his conversations container
6) bob creates his own conversation_alice container
7) bob shares conversation_alice with alice

8) alice loads the container
9) alice adds the container hmac to her conversations container

so on login
1) load conversations container
2) loop through keys and render all last_messages
3) for every theirs_hmac, being polling

when alice hits enter on a message
1) add message to messages array in conversation_bob
2) save conversation_bob
3) render conversation

render a conversation
1) grab both mine and theirs containers
2) create conversation array
3) add both container's messages arrays to conversation array
4) sort by timestamp
5) loop through and render to screen
^ this is naive and slow, we should cache it somewhere but works for now
*/

$(document).ready(function () {
  $('#login input').first().focus();

  $('#login button').click(function () {
    $(this).addClass('clicked');
  });

  $('#login form').submit(function (e) {
    e.preventDefault();

    var $inputs = $('#login input');
    var user = $inputs.first().val();
    var pass = $inputs.last().val();
    var action = $('#login button.clicked')[0].className.split(' ')[0];
    $('#login button').removeClass('clicked');

    if (action == 'login') {
      app.login(user, pass);
    } else {
      app.register(user, pass, function (err) {
        if (err) {
          app.setLoginStatus(err);
          return;
        }

        app.login(user, pass);
      });
    }
  });
});

var app = {};
app.peers = {};

app.setLoginStatus = function (m) {
  $('#login .status').text(m);
};

app.login = function (user, pass) {
  app.setLoginStatus('Logging in...');

  crypton.authorize(user, pass, function (err, session) {
    if (err) {
      app.setLoginStatus(err);
      return;
    }

    app.session = session;
    app.boot();
  });
};

app.register = function (user, pass, callback) {
  app.setLoginStatus('Generating account...');

  crypton.generateAccount(user, pass, function (err) {
    callback(err);
  });
};

app.boot = function () {
  $('#login').fadeOut(function () {
    $('#app').addClass('active');
    app.getConversations(function () {
      app.renderConversations(function () {
        $('#startConversation input').focus();
        app.bind();
      });
    });
  });

  app.session.on('message', function (message) {
    app.handleMessage(message);
  });

  app.inboxPollInterval = setInterval(function () {
    app.session.inbox.poll(function (err, messages) {
      // messages may not catch those from the initial inbox.poll()
      // when the session was created so let's always just loop through all

      for (var i in app.session.inbox.messages) {
        app.handleMessage(app.session.inbox.messages[i]);
      }
    });
  }, 2000);
};

app.handleMessage = function (message) {
  if (!message) {
    return;
  }

  if (message.headers.action != 'containerShare') {
    return;
  }

  // assuming the shared container is meant for this application
  var username = message.payload.fromUsername;
  var containerNameHmac = message.payload.containerNameHmac;

  if (app.conversations.keys[username]) {
    app.conversations.keys[username].theirs = containerNameHmac;
    app.conversations.save(function (err) {
      if (err) {
        console.log(err);
        return;
      }

      app.openConversation(username);
    });
  } else {
    // user has initiated a conversation with us
    app.getPeer(username, function (err, peer) {
      if (err) {
        alert(err);
        return;
      }

      app.createConversation(peer, function (err) {
        if (err) {
          alert(err);
          return;
        }

        app.conversations.keys[username].theirs = containerNameHmac;
        app.conversations.save(function (err) {
          if (err) {
            console.log(err);
            return;
          }

          app.renderConversations(function () {
            app.openConversation(username);
          });
        });
      });
    });
  }

  app.session.inbox.delete(message.messageId, function (err) {
    if (err) {
      console.log(err);
    }
  });
};

app.bind = function () {
  $('#header a').click(function (e) {
    e.preventDefault();
    var action = $(this).attr('data-action');

    switch (action) {
      case 'startConversation':
        $('#conversation').removeClass('active');
        $('#startConversation').removeClass('hidden');
        $('#startConversation input').focus();
        break;
    }
  });

  $('#startConversation form').submit(function (e) {
    e.preventDefault(e);
    var $input = $('#startConversation input');
    var username = $input.val();

    if (!username) {
      $input.focus();
      return;
    }

    if (app.conversations.keys[username]) {
      return app.openConversation(username);
    }

    app.getPeer(username, function (err, peer) {
      if (err) {
        alert(err);
        return;
      }

      app.createConversation(peer, function (err) {
        if (err) {
          alert(err);
          return;
        }

        app.renderConversations(function () {
          app.openConversation(username);
        });
      });
    });
  });

  $('#chatInput').keydown(function (e) {
    if (e.keyCode == 13) { // enter
      var message = $(this).val();
      app.sendMessage(message);
      $(this).val('');
    }
  });
};

app.getConversations = function (callback) {
  app.session.load('conversations', function (err, container) {
    if (err == 'No new records') {
      return app.session.create('conversations', function (err, container) {
        app.conversations = container;
        callback();
      });
    }

    app.conversations = container;
    callback();
  });
};

app.renderConversations = function (callback) {
  var $conversations = $('#sidebar #conversations');
  var conversations = Object.keys(app.conversations.keys);

  conversations.sort(function (a, b) {
    var alms = a.lastMessage && a.lastMessage.timestamp || 0;
    var blms = b.lastMessage && b.lastMessage.timestamp || 0;
    return alms - blms;
  });

  $conversations.html('');

  if (!conversations.length) {
    $('<div />')
      .addClass('noConversations')
      .text('No conversations')
      .appendTo($conversations);
    return callback();
  }

  for (var i in conversations) {
    var username = conversations[i];
    var lastMessage = app.conversations.keys[username].lastMessage || {};
    var $conversation = $('<div />').addClass('conversation');
    $conversation.attr('data-timestamp', lastMessage.timestamp);
    $('<span />').addClass('username').text(username).appendTo($conversation);
    $('<span />').addClass('message').text(lastMessage.text).appendTo($conversation);
    $conversation.appendTo($conversations);
  }

  $('#conversations .conversation').click(function (e) {
    e.preventDefault();
    var username = $(this).find('.username').text();
    app.openConversation(username);
  });

  callback && callback();
};

app.getPeer = function (username, callback) {
  if (app.peers[username]) {
    return callback(null, app.peers[username]);
  }

  app.session.getPeer(username, function (err, peer) {
    if (err) {
      return callback(err);
    }

    app.peers[username] = peer;
    callback(null, peer);
  });
};

app.createConversation = function (peer, callback) {
  app.session.create('conversation_' + peer.username, function (err, container) {
    if (err) {
      console.log(err);
      return callback(err);
    }

    container.keys['messages'] = [];

    container.save(function (err) {
      if (err) {
        console.log(err);
        return callback(err);
      }

      app.conversations.keys[peer.username] = {
        lastMessage: {
          timestamp: +new Date(),
          action: 'start',
          text: app.session.account.username + ' started a conversation'
        }
      };
      
      app.conversations.save(function (err) {
        if (err) {
          console.log(err);
          return callback(err);
        }

        container.share(peer, function (err) {
          if (err) {
            console.log(err);
            return callback(err);
          }
          
          callback();
        }); 
      });
    });
  });
};

app.openConversation = function (username) {
  $('#startConversation').addClass('hidden');
  $('#conversation').addClass('active');
  $('#chatInput').val('').focus();

  if (app.conversation && app.conversation.theirs) {
    app.conversation.theirs.unwatch();
  }

  app.session.load('conversation_' + username, function (err, ourContainer) {
    if (err) {
      console.log(err);
      return;
    }

    app.conversation = {
      mine: ourContainer,
      username: username
    };

    // grab their messages if we have access
    var theirsHmac = app.conversations.keys[username].theirs;
    if (!theirsHmac) {
      return app.renderConversation();
    } else {
      app.session.getPeer(username, function (err, peer) {
        app.session.loadWithHmac(theirsHmac, peer, function (err, theirContainer) {
          if (err) {
            console.log(err);
            return;
          }

          theirContainer.watch(function () {
            app.renderConversation();
          });

          app.conversation.theirs = theirContainer;
          app.renderConversation();
        });
      });
    }
  });
};

app.renderConversation = function () {
  var $messages = $('#conversation #messages');
  $messages.html('');

  app.conversation.messages = [];

  for (var i in app.conversation.mine.keys.messages) {
    var message = app.conversation.mine.keys.messages[i];
    message.username = app.session.account.username;
    app.conversation.messages.push(message);
  }

  if (app.conversation.theirs) {
    for (var i in app.conversation.theirs.keys.messages) {
      var message = app.conversation.theirs.keys.messages[i];
      message.username = app.conversation.username;
      app.conversation.messages.push(message);
    }
  }

  app.conversation.messages.sort(function (a, b) {
    return a.timestamp - b.timestamp;
  });

  for (var i in app.conversation.messages) {
    var message = app.conversation.messages[i];
    var $message = $('<div />').addClass('message');
    $('<span />').addClass('username').text(message.username).appendTo($message);
    $('<span />').addClass('text').text(message.text).appendTo($message);
    $message.appendTo($messages);
  }

  app.conversations.keys[app.conversation.username].lastMessage = app.conversation.messages[app.conversation.messages.length - 1];
  app.conversations.save(function (err) {
    app.renderConversations();
  });
};

app.sendMessage = function (text) {
  if (!text) {
    console.log('blank message');
    return;
  }

  var message = {
    timestamp: +new Date(),
    action: 'chat',
    text: text
  };
  
  var username = app.conversation.username;
  app.conversations.keys[username].lastMessage = message;
  app.conversations.save(function (err) {
    if (err) {
      console.log(err);
      return;
    }

    app.conversation.mine.keys.messages.push(message);
    app.conversation.mine.save(function (err) {
      if (err) {
        console.log(err);
        return;
      }

      app.renderConversation();
    });
  });
};

