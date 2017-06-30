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

var connect = require('./').connect;

/**!
 * ### saveAccount(account, callback)
 * Create account and base_keyring rows with data
 * and add account row with base_keyring_id
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} account
 * @param {Function} callback
 */
exports.saveAccount = function saveAccount(account, callback) {
  var requiredFields = [
    'username',
    'keypairCiphertext',
    'keypairMac',
    'keypairSalt',
    'keypairMacSalt',
    'signKeyPrivateMacSalt',
    'pubKey',
    'containerNameHmacKeyCiphertext',
    'hmacKeyCiphertext',
    'srpVerifier',
    'srpSalt',
    'signKeyPub',
    'signKeyPrivateCiphertext',
    'signKeyPrivateMac',
  ];

  for (var i in requiredFields) {
    if (!account[requiredFields[i]]) {
      callback('Missing field: ' + requiredFields[i]);
      return;
    }
  }

  connect(function(client, done) {
    client.query('begin');

    var checkAccountFreeQuery = {
      text: 'select username from account where username = $1',
      values: [
        account.username,
      ],
    };

    client.query(checkAccountFreeQuery, function(err, result) {
      if (err) {
        client.query('rollback');
        done();
        return callback(err);
      }

      if (!result.rowCount) {
        // No other account by this name: proceed
        var accountQuery = {
          text: '\
          insert into account (username, base_keyring_id) \
          values ($1, nextval(\'version_identifier\')) \
          returning account_id, base_keyring_id',
          values: [
            account.username,
          ],
        };

        client.query(accountQuery, function(err, result) {
          if (err) {
            client.query('rollback');
            done();

            if (err.code === '23505') {
              callback('Username already taken.');
            } else {
              logger.error('Unhandled database error: ' + err);
              callback('Database error.');
            }

            return;
          }

          var keyringQuery = {
            text: '\
              insert into base_keyring ( \
              base_keyring_id, account_id, \
              keypair, keypair_salt, keypair_mac_salt, \
              keypair_mac, pubkey, \
              container_name_hmac_key, \
              hmac_key, srp_verifier, srp_salt, \
              sign_key_pub, sign_key_private_mac_salt, \
              sign_key_private_ciphertext, \
              sign_key_private_mac \
            ) values ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15)',
            values: [
              result.rows[0].base_keyring_id,
              result.rows[0].account_id,
              account.keypairCiphertext,
              account.keypairSalt,
              account.keypairMacSalt,
              account.keypairMac,
              account.pubKey,
              account.containerNameHmacKeyCiphertext,
              account.hmacKeyCiphertext,
              account.srpVerifier,
              account.srpSalt,
              account.signKeyPub,
              account.signKeyPrivateMacSalt,
              account.signKeyPrivateCiphertext,
              account.signKeyPrivateMac,
            ],
          };

          client.query(keyringQuery, function(err) {
            if (err) {
              client.query('rollback');
              done();

              if (err.code === '23514') {
                callback('Invalid keyring data.');
              } else {
                logger.error('Unhandled database error: ' + err);
                callback('Database error.');
              }

              return;
            }

            client.query('commit', function() {
              done();
              callback();
            }); // Commit
          }); // KeyRing Query
        }); // AccountQuery

      } else {
        // We found a username that is the same!
        client.query('rollback');
        done();
        callback('Username already taken.');
        return;
      } // end 'proceed' section
    }); //CheckAccountFree
  }); // Connect
};

/**!
 * ### getAccount(username, callback)
 * Retrieve account and base_keyring rows for given `username`
 *
 * Calls back with account object and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} username
 * @param {Function} callback
 */
exports.getAccount = function getAccount(username, callback) {
  connect(function(client, done) {
    var accountQuery = {
      text: '\
        select username, \
          account.account_id, base_keyring_id, \
          srp_verifier, srp_salt, \
          keypair, keypair_salt, keypair_mac_salt, \
          keypair_mac, pubkey, \
          container_name_hmac_key, hmac_key, \
          sign_key_pub, sign_key_private_mac_salt, \
          sign_key_private_ciphertext, sign_key_private_mac \
        from account left join base_keyring using (base_keyring_id) \
        where username = $1',
      values: [
        username,
      ],
    };

    client.query(accountQuery, function(err, result) {
      done();

      if (err) {
        logger.error('*** Unhandled database error: ' + err);
        callback('Database error.');
        return;
      }

      if (!result.rows.length) {
        callback('Account not found.');
        return;
      }

      callback(null, {
        username: result.rows[0].username,
        accountId: result.rows[0].account_id,
        keyringId: result.rows[0].base_keyring_id,
        keypairSalt: JSON.parse(result.rows[0].keypair_salt.toString()),
        keypairMacSalt: JSON.parse(result.rows[0].keypair_mac_salt.toString()),
        keypairCiphertext: JSON.parse(result.rows[0].keypair.toString()),
        keypairMac: result.rows[0].keypair_mac.toString(),
        pubKey: JSON.parse(result.rows[0].pubkey.toString()),
        srpVerifier: result.rows[0].srp_verifier.toString(),
        srpSalt: result.rows[0].srp_salt.toString(),
        containerNameHmacKeyCiphertext: JSON.parse(result.rows[0].container_name_hmac_key.toString()),
        hmacKeyCiphertext: JSON.parse(result.rows[0].hmac_key.toString()),
        signKeyPub: JSON.parse(result.rows[0].sign_key_pub.toString()),
        signKeyPrivateMacSalt: JSON.parse(result.rows[0].sign_key_private_mac_salt.toString()),
        signKeyPrivateCiphertext: JSON.parse(result.rows[0].sign_key_private_ciphertext.toString()),
        signKeyPrivateMac: result.rows[0].sign_key_private_mac.toString(),
      });
    });
  });
};

/**!
 * ### getAccountById(accountId, callback)
 * Retrieve account and base_keyring rows for given `id`
 *
 * Calls back with account object and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Function} callback
 */
exports.getAccountById = function getAccountById(accountId, callback) {
  connect(function(client, done) {
    var accountQuery = {
      text: '\
        select account.account_id, \
          account.username, base_keyring_id, \
          srp_verifier, srp_salt, \
          keypair, keypair_salt, keypair_mac_salt, \
          keypair_mac, pubkey, \
          container_name_hmac_key, hmac_key, \
          sign_key_pub, sign_key_private_mac_salt, \
          sign_key_private_ciphertext, sign_key_private_mac \
        from account left join base_keyring using (base_keyring_id) \
        where account.account_id = $1',
      values: [
        accountId,
      ],
    };

    client.query(accountQuery, function(err, result) {
      done();

      if (err) {
        logger.error('Unhandled database error: ' + err);
        callback('Database error.');
        return;
      }

      if (!result.rows.length) {
        callback('Account not found.');
        return;
      }

      callback(null, {
        username: result.rows[0].username,
        accountId: result.rows[0].account_id,
        keyringId: result.rows[0].base_keyring_id,
        keypairSalt: JSON.parse(result.rows[0].keypair_salt.toString()),
        keypairMacSalt: JSON.parse(result.rows[0].keypair_mac_salt.toString()),
        keypairCiphertext: JSON.parse(result.rows[0].keypair.toString()),
        keypairMac: result.rows[0].keypair_mac.toString(),
        pubKey: JSON.parse(result.rows[0].pubkey.toString()),
        srpVerifier: result.rows[0].srp_verifier.toString(),
        srpSalt: result.rows[0].srp_salt.toString(),
        containerNameHmacKeyCiphertext: JSON.parse(result.rows[0].container_name_hmac_key.toString()),
        hmacKeyCiphertext: JSON.parse(result.rows[0].hmac_key.toString()),
        signKeyPub: JSON.parse(result.rows[0].sign_key_pub.toString()),
        signKeyPrivateMacSalt: JSON.parse(result.rows[0].sign_key_private_mac_salt.toString()),
        signKeyPrivateCiphertext: JSON.parse(result.rows[0].sign_key_private_ciphertext.toString()),
        signKeyPrivateMac: result.rows[0].sign_key_private_mac.toString(),
      });
    });
  });
};

/**
 * ### saveMessage(options, callback)
 * Add row to message table for given `options.toAccount`
 *
 * Calls back with message id and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} options
 * @param {Function} callback
 */
exports.saveMessage = function(options, callback) {
  connect(function(client, done) {
    var messageQuery = {
      text: '\
        insert into message \
          (to_account_id, from_account_id, \
          headers_ciphertext, payload_ciphertext) \
          values ($1, $2, $3, $4) \
        returning message_id',
      values: [
        options.toAccountId,
        options.fromAccountId,
        options.headersCiphertext,
        options.payloadCiphertext,
      ],
    };

    client.query(messageQuery, function(err, result) {
      done();

      if (err) {
        logger.error('Unhandled database error: ' + err);
        callback('Database error.');
        return;
      }

      callback(null, result.rows[0].message_id);
    });
  });
};

/**
 * ### getUserCount(callback)
 * Get the number of registered users
 *
 * Calls back with user count and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} options
 * @param {Function} callback
 */
exports.getUserCount = function(callback) {
  connect(function(client, done) {
    var userCountQuery = {
      text: 'select count(*) from account',
    };

    client.query(userCountQuery, function(err, result) {
      done();

      if (err) {
        logger.error('Unhandled database error: ' + err);
        return callback('Database error.');
      }

      callback(null, result.rows[0].count);
    });
  });
};

/**!
 * ### updateKeyring(account, callback)
 * Update base_keyring rows with new data after passphrase change
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} keyring
 * @param {Function} callback
 */
exports.updateKeyring = function updateKeyring(keyring, callback) {
  var requiredFields = [
    'accountId',
    'keypairCiphertext',
    'keypairMac',
    'keypairSalt',
    'keypairMacSalt',
    'signKeyPrivateMacSalt',
    'srpVerifier',
    'srpSalt',
    'signKeyPrivateCiphertext',
    'signKeyPrivateMac',
    'containerNameHmacKeyCiphertext',
    'hmacKeyCiphertext',
  ];

  for (var i in requiredFields) {
    if (!keyring[requiredFields[i]]) {
      callback('Missing field: ' + requiredFields[i]);
      return;
    }
  }

  connect(function(client, done) {
    var keyringQuery = {
      text: '\
      update base_keyring \
      set keypair = $2, \
      keypair_salt = $3, \
      keypair_mac_salt = $4, \
      keypair_mac = $5, \
      container_name_hmac_key = $6, \
      hmac_key = $7, \
      srp_verifier = $8, \
      srp_salt = $9, \
      sign_key_private_mac_salt = $10, \
      sign_key_private_ciphertext = $11, \
      sign_key_private_mac = $12 \
      where base_keyring.account_id = $1',
      values: [
        keyring.accountId,
        keyring.keypairCiphertext,
        keyring.keypairSalt,
        keyring.keypairMacSalt,
        keyring.keypairMac,
        keyring.containerNameHmacKeyCiphertext,
        keyring.hmacKeyCiphertext,
        keyring.srpVerifier,
        keyring.srpSalt,
        keyring.signKeyPrivateMacSalt,
        keyring.signKeyPrivateCiphertext,
        keyring.signKeyPrivateMac,
      ],
    };

    client.query(keyringQuery, function(err) {
      if (err) {
        client.query('rollback');
        done();

        if (err.code === '23514') {
          callback('Invalid keyring data.');
        } else {
          logger.error('Unhandled database error: ' + err);
          callback('Database error.');
        }

        return;
      }

      client.query('commit', function() {
        done();
        callback();
      });
    });
  });
};
