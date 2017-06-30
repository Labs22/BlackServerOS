var app = {};

app.init = function (session) {
  app.session = session;
  app.peers = {};

  $('#header').css({
    top: 0
  });

  $('#sidebar').css({
    left: 0
  });

  $('#container').css({
    left: '300px'
  });

  app.setUsername();

  async.series([
    app.loadMetadata,
    app.syncMessages,
    app.syncPeers,
    app.renderSidebar,
    app.bind
  ], function (err) {
    if (err) {
      alert(err);
      return;
    }

    app.setStatus('Ready');

    if (Object.keys(app.conversations).length) {
      var latestActivity = 0;
      var username;

      for (var i in app.conversations) {
        var conversation = app.conversations[i];

        if (+new Date(conversation.lastActivity) > latestActivity) {
          username = conversation.username;
        }
      }

      app.loadConversation(username);
    } else {
      app.createConversation();
    }
  });

  app.session.on('message', function (data) {
    app.session.inbox.get(data.messageId, function (err, message) {
      if (message.headers.app == 'chatExample') {
        app.processMessage(message);
        app.session.inbox.delete(data.messageId, function (err) {
          if (err) {
            alert(err);
          }
        });
      }
    });
  });
};

app.processMessage = function (message) {
  var type = message.headers.type;
  var from = message.headers.from;

  if (type == 'message') {
    if (app.conversations[from]) {
      app.conversations[from].lastActivity = message.created;
      var currentConversationWith = app.conversation && app.usernameFromConversation();

      if (currentConversationWith != from) {
        app.loadConversation(from, callback);
      } else {
        callback();
      }
    } else {
      app.startConversation(from, callback);
    }
  }

  function callback () {
    app.metadata.save(function () {
      app.renderSidebar(function () {
        app.addMessageToConversation(message, function () {
          // delete message
        });
      });
    });
  }
};

app.addMessageToConversation = function (message, callback) {
  app.conversation.get('messages', function (err, messages) {
    var messageToStore = {
      from: message.headers.from,
      body: message.payload,
      sent: +new Date(message.creationTime)
    };

    messages[message.messageId] = messageToStore;

    app.conversation.save(function () {
      app.renderMessage(messageToStore);
      callback && callback();
    });
  });
};

app.setUsername = function () {
  $('#nav .username').text(app.session.account.username);
};

app.setStatus = function (message) {
  $('#header .status').text(message);
};

app.loadMetadata = function (callback) {
  app.setStatus('Loading metadata from server...');

  app.session.load('chatMetadata', function (err, metadata) {
    if (err) {
      console.log(err);
      app.session.create('chatMetadata', function (err, metadata) {
        if (err) {
          console.log(arguments);
          return;
        }

        grabKey(metadata);
      });
      return;
    }

    grabKey(metadata);

    function grabKey (metadata) {
      metadata.add('conversations', function () {
        metadata.get('conversations', function (err, conversations) {
          app.metadata = metadata;
          app.conversations = conversations;
          callback();
        });
      });
    }
  });
};

app.syncMessages = function (callback) {
  app.setStatus('Syncing messages from server...');
  // get all messages
  // if they are applicable,
    // process into container
    // delete
  // update ui
  callback();
};

app.syncPeers = function (callback) {
  app.setStatus('Syncing peers from server...');

  async.each(Object.keys(app.conversations), function (username, cb) {
    app.addPeer(username, cb);
  }, function (err) {
    callback(err);
  });
};

app.renderSidebar = function (callback) {
  var currentConversationWith = app.conversation && app.usernameFromConversation();
  var $sidebar = $('#sidebar');
  $sidebar.html('');

  if (!Object.keys(app.conversations).length) {
    $('<p />')
      .addClass('no-conversations')
      .text('There are no existing conversations')
      .appendTo($sidebar);
  }

  for (var i in app.conversations) {
    var conversation = app.conversations[i];
    var $conversation = $('<div />').addClass('conversation');

    if (currentConversationWith == conversation.username) {
      $conversation.addClass('active');
    }

    $('<span />').addClass('username').text(conversation.username).appendTo($conversation);
    $('<span />').addClass('activity')
      .attr('data-livestamp', conversation.lastActivity)
      .livestamp(new Date(conversation.lastActivity)) // doesn't like millisecond
      .appendTo($conversation);

    $conversation.appendTo($sidebar);

    $conversation.click(function () {
      $('#messages').html('');
      // can't use conversation.username in callback without IIFE
      var username = $(this).find('.username').text();
      app.loadConversation(username);
    });
  }

  callback();
};

app.bind = function (callback) {
  $('#nav a').click(function (e) {
    e.preventDefault();
    var action = $(this).attr('data-action');
    app[action]();
  });

  callback();
};

app.createConversation = function () {
  $('#conversation').hide();
  $('#create-conversation').fadeIn();
  var input = $('#create-conversation input')[0];
  input.value = '';
  input.focus();

  $('#create-conversation form').submit(function (e) {
    e.preventDefault();
    var data = $(this).serializeArray();
    var username = data[0].value;

    if (!username) {
      return;
    }

    if (app.conversations[username]) {
      app.loadConversation(username);
      return;
    }

    app.startConversation(username);
  });
};

app.startConversation = function (username, callback) {
  app.addPeer(username, function (err) {
    if (err) {
      alert(err);
      return;
    }

    var conversation = {
      username: username,
      lastActivity: +new Date()
    };

    app.conversations[username] = conversation;

    app.metadata.save(function (err) {
      app.renderSidebar(function () {
        app.loadConversation(username, callback);
      });
    });
  });
};

app.addPeer = function (username, callback) {
  app.session.getPeer(username, function (err, peer) {
    if (err) {
      callback(err);
      return;
    }

    app.peers[username] = peer;
    callback();
  });
};

app.loadConversation = function (username, callback) {
  var containerName = 'conversation_' + username;

  app.session.load(containerName, function (err, conversation) {
    if (err) {
      console.log(err);
      app.session.create(containerName, function (err, conversation) {
        if (err) {
          console.log(err);
        }

        grabMessages(conversation);
      });
      return;
    }

    grabMessages(conversation);
  });

  function grabMessages (conversation) {
    app.conversation = conversation;
    conversation.add('messages', function () {
      conversation.get('messages', function (err, messages) {
        app.renderConversation(messages);

        callback && callback();
      });
    });
  }
};

app.usernameFromConversation = function () {
  var conversationWith = app.conversation.name.split('_')[1];
  return conversationWith;
};

app.renderConversation = function (messages) {
  $('#create-conversation').hide();

  // TODO there's a better way to do this, let's just pass it in
  var conversationWith = app.usernameFromConversation();
  var $conversations = $('#sidebar .conversation');
  $conversations.removeClass('active');
  $conversations.each(function () {
    var username = $(this).find('.username').text();
    if (username == conversationWith) {
      $(this).addClass('active');
    }
  });

  var $messages = $('#conversation #messages');
  $messages.html();

  for (var i in messages) {
    app.renderMessage(messages[i]);
  }

  $('#conversation').show();
  $('#conversation-input input').val('').focus();

  $('#conversation-input').focus().submit(function (e) {
    e.preventDefault();
    var message = $(this).serializeArray()[0].value;
    if (message) {
      $(this).find('input').val('');
      app.sendMessage(message);
    }
  });
};

app.renderMessage = function (message) {
  var $messages = $('#messages');

  var $message = $('<div />').addClass('message');
  $('<span />').addClass('username').text(message.from).appendTo($message);
  $('<span />').addClass('body').text(message.body).appendTo($message);
  $('<span />').addClass('sent')
    .attr('data-livestamp', message.sent)
    .livestamp(new Date(message.sent)) // doesn't like millisecond
    .appendTo($message);

  $message.appendTo($messages);

  $('#conversation').scrollTop($messages.height());
};

app.sendStatus = function (message) {
  var conversationWith = app.usernameFromConversation();
  var peer = app.peers[conversationWith];
  console.log(peer, message);
};

app.sendMessage = function (message) {
  var conversationWith = app.usernameFromConversation();
  var peer = app.peers[conversationWith];

  var headers = {
    app: 'chatExample',
    type: 'message',
    from: app.session.account.username
  };

  var payload = message;

  app.conversations[conversationWith].lastActivity = +new Date();
  app.conversation.get('messages', function (err, messages) {
    peer.sendMessage(headers, payload, function (err, messageId) {
      var messageToStore = {
        from: app.session.account.username,
        body: message,
        sent: +new Date()
      };

      messages[messageId] = messageToStore;

      app.conversation.save(function () {
        app.metadata.save(function () {
          app.renderSidebar(function () {
            app.renderMessage(messageToStore);
          });
        });
      });
    });
  });
};

app.logout = function () {
  app.session = null;
  app.conversations = null;

  $('#sidebar').html('');
  $('#messages').html('');

  $('#login .status').text('Logged out');

  $('body').css({
    background: '#6ab06e'
  });

  $('#header').css({
    top: '-50px'
  });

  $('#sidebar').css({
    left: '-350px'
  });

  $('#container').css({
    left: '10000px'
  });

  $('#login').css({
    top: 0
  });

  $('#login input')[0].focus();
};
