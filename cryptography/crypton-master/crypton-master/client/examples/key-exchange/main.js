/* Crypton Client, Copyright 2014 SpiderOak, Inc.
 *
 * This file is part of Crypton Server.
 *
 * Crypton Server is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Server is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Server.  If not, see <http://www.gnu.org/licenses/>.
*/

var app = {};

$(document).ready(function () {
  app.card = new crypton.Card();

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
        app.login(user, pass, true);
      });
    }
  });
});

app.setLoginStatus = function (m) {
  $('#login .status').text(m);
};

app.login = function (user, pass, isNewAccount) {
  app.setLoginStatus('Logging in...');

  crypton.authorize(user, pass, function (err, session) {
    if (err) {
      app.setLoginStatus(err);
      return;
    }
    app.session = session;
    app.boot(isNewAccount);
  });
};

app.register = function (user, pass, callback) {
  app.setLoginStatus('Generating account...');

  crypton.generateAccount(user, pass, function (err) {
    callback(err);
  });
};

app.peers = {};

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

app.boot = function (isNewAccount) {
  if (isNewAccount) {
    // show a dialog on how to share a fingerprint to exchange keys
    app.displayFingerprintInstructions(app.session.account.fingerprint,
                                       app.session.account.username);
  }

  $('#login').hide();
  $('#app').show();

  // set events
  app.bind();
};

app.displayFingerprintInstructions = function (fingerprint, username) {
  var html = '<div class="modal-dialog">'
             + '<p class="modal-content">'
             + '<p class="identicon-header">'
             + '<span class="modal-content-header">Your Fingerprint:</span>'
             + '</p>'
             + '<p id="placeholder"></p>'
             + '<div class="instruction">'
             + '<p>'
             + 'In order to share messages and data with others, you must first share your '
             + 'Fingerprint, Account ID and this application with others. '
             + '</p>'
             + '<p>Your Identigrid (color blocks) is also helpful to your friend as it is a graphic '
             + 'human recognizable representation of your Fingerprint. '
             + '</p>'
             + '<p>'
             + 'You should download this Fingerprint and send it to your friend. You have to deliver it to your friend via an out of band mechanism like email or SMS.'
             + '</p>'
             + '<p>'
             + 'Your friend will need to install this application, create an account, '
             + 'lookup your account ID and verify the Fingerprint the '
             + 'application provides against this one.'
             + '</p>'
             + '<p>'
             + 'You will also verify their Fingerprint. '
             + 'Only then can you safely exchange data or messages.'
             + '</p>'
             + '</div>'
             + '<button class="modal-dialog-close">close</button>'
             + '</p>'
             + '</div>';
  var dialog = $(html);

  $('#app').prepend(dialog);

  var canvas =
    app.card.createIdCard(fingerprint, username, app.APPNAME, 'myapp.com');
  $('#placeholder').append(canvas);

  // Make it downloadable
  var link = $('<p><a id="download-identigrid">Download Fingerprint Image</a></p>');
  // XXXddahl: add another link to download just the QR code by itself
  //           for a script to parse via: https://github.com/LazarSoft/jsqrcode
  $('#placeholder').append(link);

  document.getElementById('download-identigrid').
    addEventListener('click', function() {
    var filename = username + '-' + app.SHORTAPPNAME + '-identigrid.png';
    app.downloadCanvas(this, 'id-card', filename);
  }, false);

  $('.modal-dialog-close').click(function (){ $('.modal-dialog').remove(); });
};

app.APPNAME = 'Crypton Account Verifier';

app.SHORTAPPNAME = 'account-verifier';

app.url = 'https://localhost/';

app.generateQRCodeInput = function (fingerprint, username, application, url) {
  var json = JSON.stringify({ fingerprint: fingerprint, username: username,
                              application: application, url: url });
  return json;
};


app.dismissModalDialog = function () {
  $('.modal-dialog').remove();
};

app.bind = function () {
  var RETURN_KEY = 13;

  $('#fingerprint-instructions').click(function (){
    app.displayFingerprintInstructions(app.session.account.fingerprint,
                                       app.session.account.username);
  });

  $('#my-contacts').click(function (){
    app.displayContacts();
  });

  $('#find-someone-btn').click(function () { app.findSomeone(); });

  $('#find-someone').keyup(
    function (event) {
      var keycode = (event.keyCode ? event.keyCode : event.which);
      if (event.target == $('#find-someone')[0]) {
        if(keycode == RETURN_KEY){
          $('#find-someone-btn').focus();
	  app.findSomeone();
        }
      }
    });
};

app.findSomeone = function () {
  var username = $('#find-someone').val();
  if (!username) {
    var errtxt = "Please enter a username";
    console.error(errtxt);
    alert(errtxt);
    $('#find-someone-btn').focus();
    return;
  }

  app.getPeer(username, function (err, peer){
    if (err) {
      return alert(err);
    }
    var fingerprint = peer.fingerprint;
    app.displayPeerFingerprint(peer.username, fingerprint);
  });
};

app.displayContacts = function () {
  $('#contacts-list').remove();
  app.getContactsFromServer(function (err, contacts) {
    if (err) {
      console.error(err);
      alert('Error: cannot get contacts from server');
    }
    var html = '<table id="contacts-list">'
             + '<tr class="contacts-header">'
             + '<td>Username</td>'
             + '<td>&nbsp;</td>'
             + '<td>Fingerprint</td>'
             + '<td>Verified On</td>'
             + '</tr>'
             + '</table>';
    var list = $(html);
    $('#contacts').append(list);
    for (var prop in contacts) {
      list.append(app.formatContact(prop, contacts[prop]));
    }
  });
};

app.formatContact = function (username, metaData) {
  var html = '<tr class="contact-item">'
             + '<td>'
             + username
             + '</td>'
             + '<td id="' + username  + '">'
             + '<button id="' + username  + '-btn">View Fingerprint</button>'
             + '</td>'
             + '<td>'
             + app.card.createFingerprintArr(metaData.fingerprint).join(" ")
             + '</td>'
             + '<td>'
             + new Date(metaData.trustedAt)
             + '</td>'
             + '</tr>';
  var node = $(html);

  $('#contacts-list').append(node);
  $('#' + username + '-btn').click(function (){
    app.displayPeerFingerprint(username, metaData.fingerprint, true);
  });
};

app.downloadCanvas = function (link, canvasId, filename) {
  link.href = document.getElementById(canvasId).toDataURL();
  link.download = filename;
}

app.getContactsFromServer = function (callback) {
  app.session.load(crypton.trustStateContainer, function (err, rawContainer) {
    if (err) {
      console.error(err);
      return callback(err);
    }
    app.contactsContainer = rawContainer;
    callback(null, app.contactsContainer.keys);
  });
};

app.displayPeerFingerprint = function (username, fingerprint, isTrusted) {
  $('#peer-fingerprint').remove();
  var trusted;
  if (isTrusted) {
    trusted = true;
  } else {
    if (app.peers[username]) {
      trusted = app.peers[username].trusted;
    }
  }

  var html = '<div id="peer-fingerprint" class="modal-dialog">'
           + '<p class="modal-content">'
           + '<div class="modal-content-header">Username: '
           + '<span>'
           + username
           + '</span>'
           + '<br />'
           + '<div id="placeholder"></div>';
  if (trusted){
    html = html + '<span id="trusted-peer-label">VERIFIED PEER</span>';
  } else {
    html = html + '<button id="verify-btn">Verify User</button>';
  }
  html = html + '</div>'
       + '<button class="modal-dialog-close">close</button>'
       + '</p>'
       + '</div>';
  var dialog = $(html);
  var identigrid =
    app.card.createIdCard(fingerprint, username, app.APPNAME, app.url);
  // Make it downloadable
  var link = $('<p><a id="download-identigrid">Download Fingerprint Image</a></p>');
  // XXXddahl: add another link to download just the QR code by itself
  //           for a script to parse via: https://github.com/LazarSoft/jsqrcode
  dialog.append(link);

  $('#app').prepend(dialog);
  $('#placeholder').append(identigrid);

  document.getElementById('download-identigrid').
    addEventListener('click', function() {
    var filename = username + '-' + app.SHORTAPPNAME + '-identigrid.png';
    app.downloadCanvas(this, 'id-card', filename);
  }, false);

  $('.modal-dialog-close').click(function (){
    $('.modal-dialog').remove();
    app._currentPeer = null;
  });

  $('#verify-btn').click(function (){
    app.verifyPeer();
  });

  app._currentPeer = username;
};

app.verifyPeer = function () {
  if (!app._currentPeer) {
    console.error("currentPeer not available");
    return;
  }
  var conf = 'Does the Fingerprint and Identigrid match the ones sent to you by '
           + app._currentPeer + '?'
           + '\n\nJust clicking \'OK\' without '
           + 'visually verifying is a cop-out!';
  if (window.confirm(conf)) {
    var peer = app.peers[app._currentPeer];
    peer.trust(function (err) {
      if (err) {
        var msg = "Peer.trust failed: " + err;
        console.error(msg);
      } else {
        alert('Peer ' + app._currentPeer + ' is now trusted ');
        $('.modal-dialog').remove();
      }
    });
  }
};
