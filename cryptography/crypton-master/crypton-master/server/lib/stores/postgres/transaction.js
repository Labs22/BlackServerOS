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
var datastore = require('./');
var connect = datastore.connect;
var pg = require('pg').native;
var fs = require('fs');
var transactionQuery = fs.readFileSync(__dirname + '/sql/transaction.sql').toString();

/**!
 * Listen for container update notifications
 *
 * Upon container records being processed in a transaction,
 * the database will NOTIFY on the container_update channel
 * with the following payload format:
 *
 * `containerNameHmac:fromAccountId:toAccountId`
 *
 * where account IDs refer to those in container_session_key_share.
 *
 * If the container record is created by the container's creator
 * every account with access to the container that isn't the creator
 * will be notified by this server if they have an active websocket
 */
(function listenForContainerUpdates () {
  logger.info('listening for container updates');

  var config = process.app.config.database;
  var client = new pg.Client(config);
  client.connect();
  client.query('listen "container_update"');

  client.on('notification', function (data) {
    if (data.channel !== 'container_update') {
      return;
    }
    logger.info('container update');

    var payload = data.payload.split(':');
    var containerNameHmac = payload[0];
    var fromAccountId = payload[1];
    var toAccountId = payload[2];

    // if a client has written to their own container we
    // won't need to let them know it was updated
    // TODO perhaps this should be disabled in the case
    // where the author edited something in a separate window
    if (fromAccountId === toAccountId) {
      return;
    }

    if (app.clients[toAccountId]) {
      app.clients[toAccountId].emit('containerUpdate', containerNameHmac);
    }
  });
})();

/**!
 * ### createTransaction(accountId, callback)
 * Retrieve all records for given `containerNameHmac`
 *
 * Calls back with transaction id and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Function} callback
 */
datastore.createTransaction = function (accountId, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'insert into transaction ("account_id") \
        values ($1) returning transaction_id',
      /*jslint multistr: false*/
      values: [ accountId ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
        callback('Database error');
        return;
      }

      callback(null, result.rows[0].transaction_id);
    });
  });
};

/**!
 * ### getTransaction(transactionId, callback)
 * Retrieve transaction ros for given `transactionId`
 *
 * Calls back with transaction data and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} transactionId
 * @param {Function} callback
 */
datastore.getTransaction = function (transactionId, callback) {
  connect(function (client, done) {
    var query = {
      text: 'select * from transaction where transaction_id = $1',
      values: [
        transactionId
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
        callback('Database error');
        return;
      }

      var row = datastore.util.camelizeObject(result.rows[0]);
      callback(null, row);
    });
  });
};

/**!
 * ### abortTransaction(transactionId, callback)
 * Mark transaction aborted for given `transactionId`
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} transactionId
 * @param {Function} callback
 */
datastore.abortTransaction = function (transactionId, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'update transaction \
          set abort_timestamp = current_timestamp \
          where transaction_id=$1;',
      /*jslint multistr: false*/
      values: [
        transactionId
      ]
    };

    client.query(query, function (err, results) {
      done();
      callback(err, results);
      // TODO why pass results back?
    });
  });
};

/**!
 * ### updateTransaction(transaction, data, callback)
 * Pass `data` off to specific chunk handler to be added to given `transaction`
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} transaction
 * @param {Object} data
 * @param {Function} callback
 */
// TODO consider reversing data and transaction arguments
// to match the transaction chunk handler methods
datastore.updateTransaction = function (transaction, data, callback) {
  var types = Object.keys(datastore.transaction);
  var type = data.type;
  var valid = ~types.indexOf(type);

  if (!valid) {
    callback('Invalid transaction type');
    return;
  }

  datastore.transaction[type](data, transaction, callback);
};

/**!
 * ### requestTransactionCommit(transactionId, accountId, callback)
 * Pass transaction to commit requester after validation is successful
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} transactionId
 * @param {Number} accountId
 * @param {Function} callback
 */
datastore.requestTransactionCommit = function (transactionId, accountId, callback) {
  connect(function (client, done) {
    datastore.getTransaction(transactionId, function (err, transaction) {
      done();

      if (!transaction.transactionId) {
        callback('Transaction does not exist');
        return;
      }

      if (accountId !== transaction.accountId) {
        callback('Transaction does not belong to account');
        return;
      }

      commit.request(transaction.transactionId, callback);
    });
  });
};

/**!
 * ### transactionIsProcessed(transactionId, callback)
 * Checks if a given transaction has been fully committed
 *
 * Calls back without error, with transaction processing finished boolean,
 * transaction commit success boolean, and any errors, if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} transactionId
 * @param {Function} callback
 */
datastore.transactionIsProcessed = function (transactionId, callback) {
  connect(function (client, done) {
    datastore.getTransaction(transactionId, function (err, transaction) {
      done();

      if (!transaction.transactionId) {
        callback('Transaction does not exist');
        return;
      }

      callback(null, !!transaction.commitFinishTime, transaction.success, transaction.errors);
    });
  });
};

datastore.transaction = {};

/**!
 * ### transaction.addContainer(data, transaction, callback)
 * Add addContainer chunk to given `transaction`
 * via transaction_add_container table
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} data
 * @param {Object} transaction
 * @param {Function} callback
 */
datastore.transaction.addContainer = function (data, transaction, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'insert into transaction_add_container \
        (transaction_id, name_hmac) values ($1, $2)',
      /*jslint multistr: false*/
      values: [
        transaction.transactionId,
        data.containerNameHmac
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);

        if (~err.message.indexOf('violates unique constraint')) {
          callback('Container already exists');
          return;
        }

        callback('Invalid chunk data');
        return;
      }

      callback();
    });
  });
};

/**!
 * ### transaction.deleteContainer(data, transaction, callback)
 * Add deleteContainer chunk to given `transaction`
 * via transaction_delete_container table
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} data
 * @param {Object} transaction
 * @param {Function} callback
 */
datastore.transaction.deleteContainer = function (data, transaction, callback) {
  connect(function (client, done) {
    var containerQuery = {
      /*jslint multistr: true*/
      text: 'insert into transaction_delete_container \
        (transaction_id, name_hmac) values ($1, $2)',
      /*jslint multistr: false*/
      values: [
        transaction.transactionId,
        data.containerNameHmac
      ]
    };

    var keyQuery = {
      /*jslint multistr: true*/
      text: 'insert into transaction_delete_container_key_share \
        (transaction_id, name_hmac) values ($1, $2)',
      /*jslint multistr: false*/
      values: [
        transaction.transactionId,
        data.containerNameHmac
      ]
    };

    client.query(containerQuery, function (err, result) {
      done();

      if (err) {
        logger.warn(err);

        if (~err.message.indexOf('violates unique constraint')) {
          callback('Container already exists');
          return;
        }

        callback('Invalid chunk data');
        return;
      }

      callback();
    });
  });
};

/**!
 * ### transaction.addContainerSessionKey(data, transaction, callback)
 * Add addContainerSessionKey chunk to given `transaction`
 * via transaction_add_container_session_key table
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} data
 * @param {Object} transaction
 * @param {Function} callback
 */
datastore.transaction.addContainerSessionKey = function (data, transaction, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'insert into transaction_add_container_session_key \
        (transaction_id, name_hmac, signature) values ($1, $2, $3)',
      /*jslint multistr: false*/
      values: [
        transaction.transactionId,
        data.containerNameHmac,
        data.signature
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
        callback('Invalid chunk data');
        return;
      }

      callback();
    });
  });
};

/**!
 * ### transaction.addContainerSessionKeyShare(data, transaction, callback)
 * Add addContainerSessionKeyShare chunk to given `transaction`
 * via transaction_add_container_session_key_share table
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} data
 * @param {Object} transaction
 * @param {Function} callback
 */
datastore.transaction.addContainerSessionKeyShare = function (data, transaction, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'insert into transaction_add_container_session_key_share \
        (transaction_id, name_hmac, session_key_ciphertext, to_account_id) \
        select $1, $2, $3, account_id from account where username = $4',
      /*jslint multistr: false*/
      values: [
        transaction.transactionId,
        data.containerNameHmac,
        JSON.stringify(data.sessionKeyCiphertext),
        data.toAccount
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
        callback('Invalid chunk data');
        return;
      }

      callback();
    });
  });
};

/**!
 * ### transaction.addContainerRecord(data, transaction, callback)
 * Add addContainerRecord chunk to given `transaction`
 * via transaction_add_container_record table
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} data
 * @param {Object} transaction
 * @param {Function} callback
 */
datastore.transaction.addContainerRecord = function (data, transaction, callback) {
  // first, verify that the container was created by interacting account
  datastore.getContainerCreator(data.containerNameHmac, function (err, containerCreatorId) {
    if (err) {
      return callback(err);
    }

    if (containerCreatorId != transaction.accountId) {
      return callback('Only container creators may add records to a container');
    }

    connect(function (client, done) {
      var query = {
        /*jslint multistr: true*/
        text: "\
          insert into transaction_add_container_record \
          (transaction_id, name_hmac, latest_record_id, \
          /*hmac, payload_iv, */payload_ciphertext) \
          values ($1, $2, $3, $4)", // decode($4, 'hex'), \
          //decode($5, 'hex'), decode($6, 'hex'))",
        /*jslint multistr: false*/
        values: [
          transaction.transactionId,
          data.containerNameHmac,
          data.latestRecordId,
          //data.hmac,
          //data.payloadIv,
          data.payloadCiphertext
        ]
      };

      client.query(query, function (err, result) {
        done();

        if (err) {
          logger.warn(err);
          callback('Invalid chunk data');
          return;
        }

        callback();
      });
    });
  });
};

/**!
 * ### transaction.deleteMessage(data, transaction, callback)
 * Add deleteMessage chunk to given `transaction`
 * via transaction_delete_message table
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} data
 * @param {Object} transaction
 * @param {Function} callback
 */
datastore.transaction.deleteMessage = function (data, transaction, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: "\
        insert into transaction_delete_message \
        (transaction_id, message_id) \
        values ($1, $2)",
      /*jslint multistr: false*/
      values: [
        transaction.transactionId,
        data.messageId,
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
        callback('Invalid chunk data');
        return;
      }

      callback();
    });
  });
};

var commit = {};

/**!
 * ### commit.request(transactionId, callback)
 * Mark commit_request_time for given `transactionId`
 * so commit troller will pick it up
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} transactionId
 * @param {Function} callback
 */
commit.request = function (transactionId, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: '\
        update transaction \
          set commit_request_time = current_timestamp \
          where transaction_id=$1;',
      /*jslint multistr: false*/
      values: [
        transactionId
      ]
    };

    client.query(query, function (err, results) {
      done();
      callback(err, results);
      // TODO why are we passing back results?
    });
  });
};

/**!
 * ### commit.troll()
 * Searches for transactions with commit requested but not started
 * and passes them to commit.finish()
 */
commit.troll = function () {
  connect(function (client, done) {
    /*jslint multistr: true*/
    var query = '\
      select * from transaction \
      where commit_request_time is not null \
      and commit_start_time is null \
      order by commit_request_time asc, \
      transaction_id asc';
      /*jslint multistr: false*/

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.error(err);
        process.exit(1);
        return;
      }

      if (result.rows.length) {
        logger.info(result.rows.length + ' transactions to commit');
        // TODO queue
        for (var i in result.rows) {
          commit.finish(result.rows[i].transaction_id);
        }
      }
    });
  });
};

/**!
 * ### commit.finish(transactionId)
 * Execute transaction SQL for given `transactionId`
 */
commit.finish = function (transactionId) {
  connect(function (client, done) {
    // TODO use hostname of node
    var tq = transactionQuery
      .replace(/\{\{hostname\}\}/gi, 'hostname')
      .replace(/\{\{transactionId\}\}/gi, transactionId);

    client.query(tq, function (err, result) {
      if (err) {
        client.query('rollback');
        logger.warn(err);
        commit.fail(transactionId, err);
      }

      done();
    });
  });
};

/**!
 * ### commit.fail(transactionId, err)
 * Mark `transactionId` as failed with SQL error `err`
 *
 * XXX there is definitely a more eloquent way to do this
 */
commit.fail = function (transactionId, err) {
  logger.warn('marking failed transaction', transactionId);

  connect(function (client, done) {
    var formattedError = JSON.stringify(err, [ 'message', 'detail', 'code' ]);
    var failQuery = {
      text: '\
        update transaction \
        set commit_finish_time = current_timestamp, \
        success = false, \
        errors = $1 \
        where transaction_id=$2',
      values: [
        formattedError,
        transactionId
      ]
    };

    client.query(failQuery, function (err, result) {
      if (err) {
        logger.warn(err);
      }

      done();
    });
  });
};

var garbage = {};

/**!
 * ### garbage.trollContainers()
 * Searches for containers marked with deletion_time
 * and passes them to garbage.destroyContainer()
 */
garbage.trollContainers = function () {
  connect(function (client, done) {
    var query = {
      text: 'select container_id, name_hmac from container where deletion_time is not null',
      values: []
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
      }

      if (!result.rows.length) {
        return;
      }

      logger.info(result.rows.length + ' containers need deletion');

      for (var i = 0; i < result.rows.length; i++) {
        garbage.destroyContainer(result.rows[i].container_id);
      }
    });
  });
};

/**!
 * ### garbage.trollMessages()
 * Searches for messages marked with deletion_time
 * and passes them to garbage.destroyMessage()
 */
garbage.trollMessages = function () {
  connect(function (client, done) {
    var query = {
      text: 'select message_id from message where deletion_time is not null',
      values: []
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
      }

      if (!result.rows.length) {
        return;
      }

      logger.info(result.rows.length + ' messages need deletion');

      for (var i = 0; i < result.rows.length; i++) {
        garbage.destroyMessage(result.rows[i].message_id);
      }
    });
  });
};

/**!
 * ### garbage.destroyContainer(containerId)
 * Execute deletion SQL for given `containerId`
 */
garbage.destroyContainer = function (containerId) {
  logger.info('destroying container with id ' + containerId);

  connect(function (client, done) {
    var containerDeletionQuery = {
      text: 'delete from container where container_id = $1',
      values: [ containerId ]
    };

    client.query(containerDeletionQuery, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
      }
    });
  });
};

/**!
 * ### garbage.destroyMessage(messageId)
 * Execute deletion SQL for given `messageId`
 */
garbage.destroyMessage = function (messageId) {
  logger.info('destroying message with id ' + messageId);

  connect(function (client, done) {
    var messageDeletionQuery = {
      text: 'delete from message where message_id = $1',
      values: [ messageId ]
    };

    client.query(messageDeletionQuery, function (err, result) {
      done();

      if (err) {
        logger.warn(err);
      }
    });
  });
};

/**!
 * Search for commits every tenth of a second
 * Search for deletions every second
 */
// TODO should we make this configurable?
setInterval(commit.troll, 100);
setInterval(garbage.trollContainers, 1000);
setInterval(garbage.trollMessages, 1000);
