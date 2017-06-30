/* Crypton Server, Copyright 2013 SpiderOak, Inc.
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

'use strict';

var app = process.app;
var middleware = require('../lib/middleware');
var verifySession = middleware.verifySession;
var Account = require('../lib/account');

/**!
 * ### GET /peer/:username
 * Retrieve public key for given `username`
*/
app.get('/peer/:username', verifySession, function (req, res) {
  var account = new Account();

  account.get(req.params.username, function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });

      return;
    }

    var peer = {
      accountId: account.accountId,
      username: account.username,
      pubKey: account.pubKey,
      signKeyPub: account.signKeyPub
    };

    res.send({
      success: true,
      peer: peer
    });
  });
});

/**!
 * ### POST /peer
 * Send a message to posted `toAccount` from current session's `accountId`
*/
app.post('/peer', verifySession, function (req, res) {

  // TODO verify non-null values
  // TODO there is a more eloquent way to write this
  var options = {
    toAccountId: req.body.toAccountId,
    fromAccountId: req.session.accountId,
    headersCiphertext: req.body.headersCiphertext,
    payloadCiphertext: req.body.payloadCiphertext
  };

  var account = new Account();
  account.accountId = options.toAccountId;

  account.sendMessage(options, function (err, messageId) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      messageId: messageId
    });
  });
});
