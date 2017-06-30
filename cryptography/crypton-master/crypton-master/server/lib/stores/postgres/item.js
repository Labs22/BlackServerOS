/* Crypton Server, Copyright 2015 SpiderOak, Inc.
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

var datastore = require('./');
var connect = datastore.connect;
var app = process.app;
var pg = require('pg').native;

/**!
 * ### getItemValue(itemNameHmac, accountId, callback)
 * Retrieve value for given `itemNameHmac` accessible by `accountId`
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} itemNameHmac
 * @param {Number} accountId
 * @param {Function} callback
 */
exports.getItemValue = function (itemNameHmac, accountId, callback) {
  var toAccountId = accountId; // The query limits the results to any user that has a valid session_key_share, so all users are considered the "toAccount"
  connect(function (client, done) {
    var query = {
      // TODO limit to to_account_id
      /*jslint multistr: true*/
      text: 'select i.item_id, i.value, i.name_hmac, i.account_id, \
             i.modified_time, sk.item_id, \
             s.session_key_ciphertext \
             from item i \
             left join item_session_key sk on i.item_id = sk.item_id \
             left join item_session_key_share s \
                       on sk.item_session_key_id = s.item_session_key_id \
             where \
             i.name_hmac = $1 and \
             i.deletion_time is null and \
             sk.supercede_time is null and \
             s.deletion_time is null and \
             s.to_account_id = $2 \
             limit 1',
      // i.account_id = $2 and \
      /*jslint multistr: false*/
      values: [
        itemNameHmac,
        // accountId,
        toAccountId
      ]
    };

    client.query(query, function (err, result) {
      done();
      if (err) {
        return callback(err);
      }

      if (result.rowCount !== 1) {
        return callback('Item does not exist');
      }

      var rawData = {
        ciphertext: JSON.parse(result.rows[0].value.toString()),
        modTime: Date.parse(result.rows[0].modified_time),
        wrappedSessionKey: result.rows[0].session_key_ciphertext
      };
      callback(null, rawData);
    });
  });
};

/**!
 * ### saveItem(itemNameHmac, accountId, value, timelineVisible, callback)
 * Save Item
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} itemNameHmac
 * @param {Number} accountId
 * @param {String} value
 * @param {Boolean} timelineVisible
 * @param {Function} callback
 */
exports.saveItem = function (itemNameHmac, accountId, value, timelineVisible, callback) {
  connect(function (client, done) {
    var updateQuery = {
      /*jslint multistr: true*/
      text: '\
        update \
        item set value = $1, timeline_visible = $2 \
        where account_id = $3 and name_hmac = $4 returning modified_time',
      /*jslint multistr: false*/
      values: [
        value,
	timelineVisible,
        accountId,
        itemNameHmac
      ]
    };

    client.query(updateQuery, function (err, result) {
      done();
      if (err) {
        return callback(err);
      }
      var modTime;
      try {
        modTime = result.rows[0].modified_time;
      } catch (ex) {
        logger.info(ex);
        logger.info(ex.stack);
      }
      callback(null, { modTime: Date.parse(modTime),
                       itemNameHmac: itemNameHmac
                     });
    });
  });
};

/**!
 * ### createItem(itemNameHmac, accountId, value, callback)
 * Save Item
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} itemNameHmac
 * @param {Number} accountId
 * @param {String} value
 * @param {Function} callback
 */
exports.createItem =
function (itemNameHmac, accountId, value, wrappedSessionKey, callback) {
  connect(function (client, done) {
    client.query('begin');
    var query = {
      /*jslint multistr: true*/
      text: '\
        insert into item (account_id, name_hmac, value) \
        values ($1, $2, $3) \
        returning item_id, modified_time',
      /*jslint multistr: false*/
      values: [
        accountId,
        itemNameHmac,
        value
      ]
    };

    client.query(query, function (err, result) {
      if (err) {
        logger.info(err);
        client.query('rollback');
        done();
        return callback(err);
      }
      var itemId = result.rows[0].item_id;
      var modTime = result.rows[0].modified_time;

      // sessionkey query
      var sessionKeyQuery = {
        text: 'insert into item_session_key \
          ( item_id, account_id ) \
          values ( $1, $2 ) returning item_session_key_id',
        values: [itemId, accountId]
      };

      client.query(sessionKeyQuery, function (err, result) {
        if (err) {
          logger.info(err);
          client.query('rollback');
          done();
          return callback(err);
        }
        var itemSessionKeyId = result.rows[0].item_session_key_id;

        // sessionkey share query
        var sessionKeyShareQuery = {
          text: 'insert into item_session_key_share \
            ( item_session_key_id, account_id, to_account_id, session_key_ciphertext  ) \
            values ( $1, $2, $3, $4 )',
          values: [itemSessionKeyId, accountId, accountId, wrappedSessionKey]
        };

        client.query(sessionKeyShareQuery, function (err, result) {
          if (err) {
            logger.info(err);
            client.query('rollback');
            done();
            return callback(err);
          }
          // success
          client.query('commit');
          done();
          var metadata = { itemNameHmac: itemNameHmac, modTime: Date.parse(modTime) };
          callback(null, metadata);
        });
      });
    });
  });
};

/**!
 * ### removeItem(itemNameHmac, accountId, value, callback)
 * Remove Item
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} itemNameHmac
 * @param {Number} accountId
 * @param {String} value
 * @param {Function} callback
 */
exports.removeItem =
function (itemNameHmac, accountId, callback) {
  connect(function (client, done) {
    client.query('begin');
    var query = {
      /*jslint multistr: true*/
      text: '\
        delete from item where name_hmac = $1 and account_id = $2',
        // All child rows in item_session_key &
        // item_session_key_share will cascade delete
      /*jslint multistr: false*/
      values: [
        itemNameHmac,
        accountId
      ]
    };

    client.query(query, function (err, result) {
      if (err) {
        logger.info(err);
	client.query('rollback');
        done();
        return callback(err);
      }
      client.query('commit');
      done();
      return callback(null, result.rows[0]);
    });
  });
};

/**!
 * ### shareItem(itemNameHmac, fromAccountId, toAccountId, callback)
 * Share Item
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} itemNameHmac
 * @param {String} sessionKeyCiphertext
 * @param {String} toUsername
 * @param {Number} fromAccountId
 * @param {Function} callback
 */
exports.shareItem =
function (itemNameHmac, sessionKeyCiphertext,
          toUsername, fromAccountId, callback) {
  var toAccountId;
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'select account_id, username \
             from account \
             where \
             username = $1',
      /*jslint multistr: false*/
      values: [
        toUsername
      ]
    };

    client.query(query, function (err, result) {
      if (err) {
        client.query('rollback');
        done();
        logger.info(err);
        return callback(err);
      }

      if (result.rowCount != 1) {
        return callback('Account with username' + toUsername  + 'does not exist');
      }
      toAccountId = result.rows[0].account_id;

      client.query('begin');
      // We have a user now, we need to insert item_session_key!
      var itemSessionKeyQuery = {
        /*jslint multistr: true*/
        text: 'select item_session_key_id from item_session_key \
               where item_id = \
               (select item_id from item where name_hmac = $1) \
               and account_id = $2;',
        /*jslint multistr: false*/
        values: [
          itemNameHmac,
          fromAccountId
        ]
      };

      client.query(itemSessionKeyQuery, function (err, result) {
        if (err) {
          client.query('rollback');
          done();
          logger.info(err);
          return callback(err);
        }

        if (result.rowCount != 1) {
	  done();
          return callback('item_session_key lookup failed');
        }

        var itemSessionKeyId = result.rows[0].item_session_key_id;
        // We have a user now, we need to insert item_session_key_share
        var itemSessionKeyShareQuery = {
          /*jslint multistr: true*/
          text: 'insert into item_session_key_share \
                 (item_session_key_id, account_id, \
                 to_account_id, session_key_ciphertext) values \
                 ($1, $2, $3, $4) \
                 returning item_session_key_share_id;',
          /*jslint multistr: false*/
          values: [
            itemSessionKeyId,
            fromAccountId,
            toAccountId,
            sessionKeyCiphertext
          ]
        };

        client.query(itemSessionKeyShareQuery, function (err, result) {
          if (err) {
            client.query('rollback');
            done();
            logger.error(err);
            return callback(err);
          }

          if (result.rowCount != 1) {
            client.query('rollback');
            done();
            return callback('item_session_key_share insert failed');
          }
          var itemSessionKeyShareId =
            result.rows[0].item_session_key_share_id;

          client.query('commit');
	  done();
          return callback(null, {success: true});
        });
      });
    });
  });
};

/**!
 * ### unshareItem(itemNameHmac, shareeAccountId, callback)
 * unshare Item
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} itemNameHmac
 * @param {Number} accountId
 * @param {Number} shareeUsername
 * @param {Function} callback
 */
exports.unshareItem =
function (itemNameHmac, accountId, shareeUsername, callback) {
  logger.info('unshareItem()');

  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'update item_session_key_share \
      set deletion_time = CURRENT_TIMESTAMP \
      where \
      item_session_key_id = \
         (select item_session_key_id from item_session_key where item_id = \
           (select item_id from item where name_hmac = decode($1, \'escape\'))) \
      and to_account_id = \
      (select account_id from account where username = $2) \
      and account_id = $3',
      /*jslint multistr: false  */
      values: [
        itemNameHmac,
        shareeUsername,
        accountId
      ]
    };

    // XXXddahl: TODO: Create update trigger on item_session_key_share
    // to remove row after deletion_time is set
    client.query(query, function (err, result) {
      done();
      if (err) {
        logger.error('ERROR UPDATING ITEM SESSION KEY SHARE');
        return callback(err);
      }
      if (result.rowCount != 1) {
        return callback('ItemSessionKeyShare delete failed');
      }
      callback(null);
    });
  });
};

/**!
 * ### getAuthorItems(accountId, offset, limit, callback)
 * Get author's items from an offset
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Number} offset
 * @param {Number} lastHistoryItemIdRead
 * @param {Number} limit
 * @param {Function} callback
 */
exports.getAuthorItems =
function getAuthorItems (accountId, lastHistoryItemIdRead, offset, limit, callback) {
  // lastHistoryItemIdRead is the item_history_id
  connect(function (client, done) {
    if (!offset) {
      offset = 0;
    }
    if (!limit) {
      limit = 10;
    }
    if (!lastHistoryItemIdRead) {
      lastHistoryItemIdRead = 0;
    }

    var query = {
      /*jslint multistr: true*/
      text: 'select i.item_id, h.item_history_id, h.value, i.name_hmac, \
             s.session_key_ciphertext, h.creation_time, h.modified_time \
             from item_history h \
             left join item i on i.item_id = h.item_id \
             left join item_session_key sk on i.item_id = sk.item_id \
             left join item_session_key_share s \
                       on sk.item_session_key_id = s.item_session_key_id \
             where \
             h.account_id = $1 and \
             h.item_history_id > $4 and \
             i.deletion_time is null and \
             sk.supercede_time is null and \
             s.deletion_time is null and \
             s.to_account_id = $1 \
             order by h.item_history_id asc \
             limit $3 offset $2',
      /*jslint multistr: false*/
      values: [
        accountId,
        offset,
        limit,
        lastHistoryItemIdRead
      ]
    };

    client.query(query, function (err, result) {
      done();
      if (err) {
        return callback(err);
      }

      var resultData = [];

      if (result.rowCount < 1) {
        return callback(null, resultData);
      }

      for (var i = 0; i < result.rows.length; i++) {
	var record = {
          ciphertext: JSON.parse(result.rows[i].value.toString()),
	  modTime: Date.parse(result.rows[i].modified_time),
          creationTime: Date.parse(result.rows[i].creation_time),
          wrappedSessionKey: result.rows[i].session_key_ciphertext,
	  itemHistoryId: result.rows[i].item_history_id,
          itemNameHmac: result.rows[i].name_hmac
	};
	resultData.push(record);
      }

      callback(null, resultData);
    });
  });
};

/**!
 * ### getTimelineItems(accountId, lastTimelineIdRead, offset, limit, callback)
 * Get user's timeline items from an offset
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Number} lastTimelineIdRead
 * @param {Number} offset
 * @param {Number} limit
 * @param {Function} callback
 */
exports.getTimelineItems =
function getTimelineItems (accountId, lastTimelineIdRead, offset, limit, direction, callback) {
  logger.info(arguments);
  connect(function (client, done) {
    if (!offset || (typeof parseInt(offset) != 'number')) {
      offset = 0;
    }
    if (!limit || (typeof parseInt(limit) != 'number')) {
      limit = 10;
    }
    if (!lastTimelineIdRead || (typeof parseInt(lastTimelineIdRead) != 'number')) {
      lastTimelineIdRead = 0;
    }
    var orderby = 'ASC';
    /*jslint multistr: true*/
    var wherePrev = ' where \
      t.receiver_id = $1 and \
      t.timeline_id < (select timeline_id from timeline where receiver_id = $1 order by timeline_id DESC LIMIT 1) and \
      i.deletion_time is null and \
      sk.supercede_time is null and \
      s.deletion_time is null and \
      s.to_account_id = $1 ';
    /*jslint multistr: false*/

    /*jslint multistr: true*/
    var whereNext = ' where \
      t.receiver_id = $1 and \
      t.timeline_id > $4 and \
      i.deletion_time is null and \
      sk.supercede_time is null and \
      s.deletion_time is null and \
      s.to_account_id = $1 ';
    /*jslint multistr: false*/

    var whereClause = whereNext;
    var prev;
    switch (direction) {
    case 'undefined':
      whereClause = whereNext;
      break;
    case 'prev':
      whereClause = wherePrev;
      prev = true;
      orderby = 'DESC';
      break;
    case 'next':
      whereClause = whereNext;
      break;
    default:
      whereClause = whereNext;
    }

    logger.info('direction in query: ', direction);

    var values = [
      accountId,
      offset,
      limit,
      lastTimelineIdRead
    ];

    if (prev) {
      values = [
        accountId,
        offset,
        limit
      ];
    }

    var query = {
      /*jslint multistr: true*/
      text: 'select i.item_id, t.timeline_id, t.creator_id, t.receiver_id, \
             t.value, t.creation_time, t.modified_time, s.session_key_ciphertext, \
             a.username as creator_username \
             from timeline t \
             left join item i on i.item_id = t.item_id \
             left join item_session_key sk on i.item_id = sk.item_id \
             left join item_session_key_share s \
                       on sk.item_session_key_id = s.item_session_key_id \
             left join account a on t.creator_id = a.account_id '
             + whereClause
             + ' order by t.timeline_id ' + orderby
             + ' limit $3 offset $2',
      /*jslint multistr: false*/
      values: values
    };
    logger.info(query.text);
    client.query(query, function (err, result) {
      done();
      if (err) {
        return callback(err);
      }

      var resultData = [];
      if (result.rowCount < 1) {
        return callback(null, resultData);
      }

      for (var i = 0; i < result.rows.length; i++) {
	var record = {
          ciphertext: JSON.parse(result.rows[i].value.toString()),
	  modTime: Date.parse(result.rows[i].modified_time),
          creationTime: Date.parse(result.rows[i].creation_time),
          wrappedSessionKey: result.rows[i].session_key_ciphertext,
	  timelineId: result.rows[i].timeline_id,
          creatorUsername: result.rows[i].creator_username
	};
	resultData.push(record);
      }

      callback(null, resultData);
    });
  });
};

/**!
 * ### getLatestTimelineItems(accountId, limit, callback)
 * Get user's latest N timeline items
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Number} limit
 * @param {Function} callback
 */
exports.getLatestTimelineItems =
  function getLatestTimelineItems (accountId, limit, callback) {
  logger.info(arguments);
  connect(function (client, done) {
    if (!limit || (typeof parseInt(limit) != 'number')) {
      limit = 10;
    }

    var query = {
      /*jslint multistr: true*/
      text: 'select i.item_id, t.timeline_id, t.creator_id, t.receiver_id, \
             t.value, t.creation_time, t.modified_time, s.session_key_ciphertext, \
             a.username as creator_username \
             from timeline t \
             left join item i on i.item_id = t.item_id \
             left join item_session_key sk on i.item_id = sk.item_id \
             left join item_session_key_share s \
             on sk.item_session_key_id = s.item_session_key_id \
             left join account a on t.creator_id = a.account_id \
	     where \
             t.timeline_id > (select timeline_id from timeline WHERE receiver_id = $1 ORDER BY timeline_id DESC limit 1 OFFSET 16) and \
	     t.receiver_id = $1 and \
             i.deletion_time is null and \
             sk.supercede_time is null and \
             s.deletion_time is null and \
             s.to_account_id = $1 \
             order by t.timeline_id DESC \
             limit $2',
      /*jslint multistr: false*/
      values: [
	accountId,
	limit
      ]
    };

    logger.info('Normal Latest Query: \n\n' + query.text);
    var resultData = [];

    client.query(query, function (err, result) {
      done();
      if (err) {
        return callback(err);
      }

      if (result.rowCount < 1) {
	// This is one of the first times a user has queried the timeline
	// We should re-run with fewer constraints

	var newUserQuery = {
	  /*jslint multistr: true*/
	  text: 'select i.item_id, t.timeline_id, t.creator_id, t.receiver_id, \
            t.value, t.creation_time, t.modified_time, s.session_key_ciphertext, \
             a.username as creator_username \
             from timeline t \
             left join item i on i.item_id = t.item_id \
             left join item_session_key sk on i.item_id = sk.item_id \
             left join item_session_key_share s \
             on sk.item_session_key_id = s.item_session_key_id \
             left join account a on t.creator_id = a.account_id \
	     where \
             t.timeline_id >= (select timeline_id from timeline WHERE receiver_id = $1 ORDER BY timeline_id DESC limit 1) and \
	     t.receiver_id = $1 and \
             i.deletion_time is null and \
             sk.supercede_time is null and \
             s.deletion_time is null and \
             s.to_account_id = $1 \
             order by t.timeline_id DESC \
             limit $2',
	  /*jslint multistr: false*/
	  values: [
	    accountId,
	    limit
	  ]
	};

  //return callback(null, resultData);
	logger.info('First Run Latest Query: \n\n' + newUserQuery.text);

	client.query(newUserQuery, function (err, result) {
	  done();
	  if (err) {
            return callback(err);
	  }

	  for (var i = 0; i < result.rows.length; i++) {
	    var record = {
              ciphertext: JSON.parse(result.rows[i].value.toString()),
	      modTime: Date.parse(result.rows[i].modified_time),
              creationTime: Date.parse(result.rows[i].creation_time),
              wrappedSessionKey: result.rows[i].session_key_ciphertext,
	      timelineId: result.rows[i].timeline_id,
              creatorUsername: result.rows[i].creator_username
	    };
	    resultData.push(record);
	  }

	  callback(null, resultData);
	});

      } else {
	// Using the original results here!
	for (var i = 0; i < result.rows.length; i++) {
	  var record = {
            ciphertext: JSON.parse(result.rows[i].value.toString()),
	    modTime: Date.parse(result.rows[i].modified_time),
            creationTime: Date.parse(result.rows[i].creation_time),
            wrappedSessionKey: result.rows[i].session_key_ciphertext,
	    timelineId: result.rows[i].timeline_id,
            creatorUsername: result.rows[i].creator_username
	  };
	  resultData.push(record);
	}

	callback(null, resultData);
      }
    });
  });
};

/**!
 * ### getTimelineItemsBefore(accountId, limit, beforeId, callback)
 * Get N timeline items before item 'beforeId'
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Number} limit
 * @param {Number} beforeId
 * @param {Function} callback
 */
exports.getTimelineItemsBefore =
  function getTimelineItemsBefore (accountId, limit, beforeId, callback) {
  logger.info(arguments);
  connect(function (client, done) {
    if (!limit || (typeof parseInt(limit) != 'number')) {
      limit = 10;
    }
    if (typeof parseInt(beforeId) != 'number') {
      done();
      return callback('beforeId argument is missing!');
    }
    var query = {
      /*jslint multistr: true*/
      text: 'select i.item_id, t.timeline_id, t.creator_id, t.receiver_id, \
             t.value, t.creation_time, t.modified_time, s.session_key_ciphertext, \
             a.username as creator_username \
             from timeline t \
             left join item i on i.item_id = t.item_id \
             left join item_session_key sk on i.item_id = sk.item_id \
             left join item_session_key_share s \
                       on sk.item_session_key_id = s.item_session_key_id \
             left join account a on t.creator_id = a.account_id \
	     where \
	     t.receiver_id = $1 and \
             t.timeline_id < $2 and \
             i.deletion_time is null and \
             sk.supercede_time is null and \
             s.deletion_time is null and \
             s.to_account_id = $1 \
             order by t.timeline_id DESC \
             limit $3',
      /*jslint multistr: false*/
      values: [
	accountId,
	beforeId,
	limit
      ]
    };
    logger.info(query.text);
    client.query(query, function (err, result) {
      done();
      if (err) {
        return callback(err);
      }

      var resultData = [];
      if (result.rowCount < 1) {
        return callback(null, resultData);
      }

      for (var i = 0; i < result.rows.length; i++) {
	var record = {
          ciphertext: JSON.parse(result.rows[i].value.toString()),
	  modTime: Date.parse(result.rows[i].modified_time),
          creationTime: Date.parse(result.rows[i].creation_time),
          wrappedSessionKey: result.rows[i].session_key_ciphertext,
	  timelineId: result.rows[i].timeline_id,
          creatorUsername: result.rows[i].creator_username
	};
	resultData.push(record);
      }

      callback(null, resultData);
    });
  });
};



/**!
 * ### getTimelineItemsAfter(accountId, limit, afterId, callback)
 * Get N timeline items after item 'afterId'
 *
 * Calls back with value and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Number} limit
 * @param {Number} afterId
 * @param {Function} callback
 */
exports.getTimelineItemsAfter =
    function getTimelineItemsAfter (accountId, limit, afterId, callback) {
  logger.info(arguments);
  connect(function (client, done) {
    if (!limit || (typeof parseInt(limit) != 'number')) {
      limit = 10;
    }

    var query = {
      /*jslint multistr: true*/
      text: 'select i.item_id, t.timeline_id, t.creator_id, t.receiver_id, \
             t.value, t.creation_time, t.modified_time, s.session_key_ciphertext, \
             a.username as creator_username \
             from timeline t \
             left join item i on i.item_id = t.item_id \
             left join item_session_key sk on i.item_id = sk.item_id \
             left join item_session_key_share s \
             on sk.item_session_key_id = s.item_session_key_id \
             left join account a on t.creator_id = a.account_id \
	     where \
	     t.receiver_id = $1 and \
             t.timeline_id > $2 and \
             i.deletion_time is null and \
             sk.supercede_time is null and \
             s.deletion_time is null and \
             s.to_account_id = $1 \
             order by t.timeline_id DESC \
             limit $3',
      /*jslint multistr: false*/
      values: [
	accountId,
	afterId,
	limit
      ]
    };
    logger.info(query.text);
    client.query(query, function (err, result) {
      done();
      if (err) {
        return callback(err);
      }

      var resultData = [];
      if (result.rowCount < 1) {
        return callback(null, resultData);
      }

      for (var i = 0; i < result.rows.length; i++) {
	var record = {
          ciphertext: JSON.parse(result.rows[i].value.toString()),
	  modTime: Date.parse(result.rows[i].modified_time),
          creationTime: Date.parse(result.rows[i].creation_time),
          wrappedSessionKey: result.rows[i].session_key_ciphertext,
	  timelineId: result.rows[i].timeline_id,
          creatorUsername: result.rows[i].creator_username
	};
	resultData.push(record);
      }

      callback(null, resultData);
    });
  });
};




/**!
 * Listen for item update notifications
 *
 * Upon item value update,
 * the database will NOTIFY on the item_update channel
 * with the following payload format:
 *
 * `itemNameHmac:fromAccountId:toAccountId`
 *
 * where account IDs refer to those in item_session_key_share.
 *
 * If the item value is created by the item's creator
 * every account with access to the item that isn't the creator
 * will be notified by this server if they have an active websocket
 * XXXddahl: If not, a message is sent
 */
(function listenForItemUpdates () {
  logger.info('listening for item updates');

  var config = process.app.config.database;
  var client = new pg.Client(config);
  client.connect();
  client.query('listen "SharedItemUpdated"');

  client.on('notification', function (data) {
    if (data.channel != 'SharedItemUpdated') {
      return;
    }
    logger.info('SharedItemUpdated', JSON.stringify(data));
    var payload = data.payload.split(' ');
    var itemNameHmac = payload[2];
    var toAccountId = payload[0];
    var fromAccountId = payload[1];
    var toUsername = payload[3];
    var creatorUsername = payload[4];

    // if a client has written to their own Item we
    // won't need to let them know it was updated
    // TODO perhaps this should be disabled in the case
    // where the author edited something in a separate window
    if (fromAccountId == toAccountId) {
      return; // XXXddahl: creator is not going to be notified for now
    }

    if (app.clients[toAccountId]) {
      app.clients[toAccountId].emit('itemUpdate',
                                    { itemNameHmac: itemNameHmac,
                                      creator: creatorUsername,
                                      toUsername: toUsername });
    } else {
      // XXXddahl: Can we store a message for later in Redis???
      logger.warn('Sharee is not connected via websocket');
      logger.warn('Connected clients: ', app.clients);
    }
  });
})();
