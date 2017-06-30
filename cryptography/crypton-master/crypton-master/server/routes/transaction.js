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
var db = app.datastore;
var config = app.config;
var verifySession = require('../lib/middleware').verifySession;
var Transaction = require('../lib/transaction');

// check manually for commit looping config variables here
// because it seems odd to define defaults for them in this file
// or in app.js where other defaults are defined, and when they
// aren't configured commit POST request will silently never respond
if (!config.commitStatusCheckLimit || !config.commitStatusCheckDelay) {
  throw 'commitStatusCheckLimit and commitStatusCheckDelay must be set in config file';
}

/**!
 * ### POST /transaction/create
 * Create a transaction for current session's `accountId`
 * and return created transaction's `transactionId`
*/
app.post('/transaction/create', verifySession, function (req, res) {
  var accountId = req.session.accountId;

  var tx = new Transaction();
  tx.create(accountId, function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      id: tx.transactionId
    });
  });
});

/**!
 * ### POST /transaction/:transactionId
 * Add posted body as a chunk to `transactionId`
*/
app.post('/transaction/:transactionId', verifySession, function (req, res) {
  var data = req.body;
  var transactionId = req.params.transactionId;
  var accountId = req.session.accountId;

  var tx = new Transaction();
  tx.get(transactionId, function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    tx.update('interactingAccount', accountId);

    tx.add(data, function (err) {
      if (err) {
        res.send({
          success: false,
          error: err
        });
        return;
      }

      res.send({
        success: true
      });
    });
  });
});

/**!
 * ### POST /transaction/:transactionId/commit
 * Request commit for given `transactionId`
*/
app.post('/transaction/:transactionId/commit', verifySession, function (req, res) {
  var transactionId = req.params.transactionId;
  var accountId = req.session.accountId;

  var tx = new Transaction();

  tx.update('interactingAccount', accountId);

  tx.get(transactionId, function (err) {
    tx.requestCommit(function (err) {
      if (err) {
        return res.send({
          success: false,
          error: err
        });
      }

      var timesChecked = 0;
      checkCommitStatus();

      function checkCommitStatus () {
        tx.isProcessed(function (err, isProcessed, success, errors) {
          if (err) {
            return res.send({
              success: false,
              error: err
            });
          }

          // if the transaction hasn't been processed yet,
          // and we haven't hit a configured limit of checks,
          // check again in the configured amount of time
          if (!isProcessed) {
            timesChecked++;

            if (timesChecked < app.config.commitStatusCheckLimit) {
              setTimeout(checkCommitStatus, app.config.commitStatusCheckDelay);
            }

            return;
          }

          // transaction has committed successfully
          if (success) {
            return res.send({
              success: true
            });
          }

          // transaction has been processed but with errors
          // XXX in the future we will need to provide the client
          // with a generalized error category so it can decide
          // what to do with itself.
          // for now we just let it know an error occurred in the commit
          res.send({
            success: false,
            error: 'Malformed transaction'
          });
        });
      }
    });
  });
});

/**!
 * ### DELETE /transaction/:transactionId
 * Abort given `transactionId`
*/
app.delete('/transaction/:id', verifySession, function (req, res) {
  var transactionId = req.params.id;
  var accountId = req.session.accountId;

  var tx = new Transaction();

  tx.update('interactingAccount', accountId);

  // FIXME : tbe outer 'err' is never handled.
  tx.get(transactionId, function (err) {
    tx.abort(function (err) {
      if (err) {
        res.send({
          success: false,
          error: err
        });
        return;
      }

      res.send({
        success: true
      });
    });
  });
});
