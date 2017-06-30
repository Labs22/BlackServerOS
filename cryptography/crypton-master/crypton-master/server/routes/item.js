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
var Item = require('../lib/item');

/**!
 * ### GET /item/:itemNameHmac
 * Retrieve item value for the given `itemNameHmac`
*/
app.get('/item/:itemNameHmac', verifySession, function (req, res) {
  var accountId = req.session.accountId;
  var itemNameHmac = req.params.itemNameHmac;

  var item = new Item();
  item.update('accountId', accountId);

  item.get(itemNameHmac, function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      rawData: item.rawData // XXXddahl: Send the whole item?
    });
  });
});

/**!
 * ### POST /item/:itemNameHmac
 * Update item value for the given `itemNameHmac`
*/
app.post('/item/:itemNameHmac', verifySession, function (req, res) {
  var item = new Item();

  var accountId = req.session.accountId;
  item.update('accountId', accountId);

  var itemNameHmac = req.body.itemNameHmac;
  item.update('itemNameHmac', itemNameHmac);

  var value = req.body.payloadCiphertext;
  item.update('value', value);

  var timelineVisible = req.body.timelineVisible;
  item.update('timelineVisible', timelineVisible);

  item.save(function (err, result) {
    if (err) {
      res.send({
        success: false,
        error: err
      });

      return;
    }
    res.send({
      success: true,
      result: result
    });
  });
});

/**!
 * ### POST /createitem
 * Create item value for the given `itemNameHmac`
*/
app.post('/createitem', verifySession, function (req, res) {
  var item = new Item();

  var accountId = req.session.accountId;
  item.update('accountId', accountId);

  var itemNameHmac = req.body.itemNameHmac;
  item.update('itemNameHmac', itemNameHmac);

  var wrappedSessionKey = req.body.wrappedSessionKey;
  item.update('wrappedSessionKey', wrappedSessionKey);

  var value = req.body.payloadCiphertext;
  item.update('value', value);

  // Make sure client sends all correct arguments
  if (!(value && wrappedSessionKey && itemNameHmac && accountId)) {
    res.send({
      success: false,
      error: '/createitem: missing argument error'
    });
    return;
  }

  item.create(function (err, metaData) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      itemMetaData: metaData
    });
  });
});

/**!
 * ### POST /removeitem
 * Remove item for the given `itemNameHmac`
*/
app.post('/removeitem', verifySession, function (req, res) {
  var item = new Item();

  var accountId = req.session.accountId;
  if (!accountId) {
    res.send({
      success: false,
      error: 'Missing accountId in POST'
    });
  }
  item.update('accountId', accountId);

  var itemNameHmac = req.body.itemNameHmac;
  if (!itemNameHmac) {
    res.send({
      success: false,
      error: 'Missing itemNameHmac in POST'
    });
  }
  item.update('itemNameHmac', itemNameHmac);

  item.remove(function (err, result) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      itemMetaData: result
    });
  });
});

/**!
 * ### POST /shareitem/:itemNameHmac
 * Share item for the given `itemNameHmac` with peer
*/
app.post('/shareitem/:itemNameHmac', verifySession, function (req, res) {
  var accountId = req.session.accountId;
  var itemNameHmac = req.params.itemNameHmac;

  var item = new Item();

  var toUsername = req.body.toUsername;
  item.update('toUsername', toUsername);

  var sessionKeyCiphertext = req.body.sessionKeyCiphertext;
  item.update('sessionKeyCiphertext', sessionKeyCiphertext);

  item.update('accountId', accountId);
  item.update('itemNameHmac', itemNameHmac);

  item.share(function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      error: null
    });
  });
});

/**!
 * ### POST /unshareitem/:itemNameHmac
 * unshare item for the given `itemNameHmac` from peer
*/
app.post('/unshareitem/:itemNameHmac', verifySession, function (req, res) {
  var accountId = req.session.accountId;
  var itemNameHmac = req.params.itemNameHmac;

  var item = new Item();

  var shareeUsername = req.body.shareeUsername;
  item.update('shareeUsername', shareeUsername);

  item.update('accountId', accountId);
  item.update('itemNameHmac', itemNameHmac);

  item.unshare(function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      error: null
    });
  });
});

/**!
 * ### GET /itemhistory/
 * Retrieve item history
*/
app.get('/itemhistory/', verifySession, function (req, res) {
  var accountId = req.session.accountId;
  var lastItemRead = req.query.historyid || 0;
  var offset = req.query.offset || 0;
  // set max limit of 100
  var limit = 10;
  if (req.query.limit > 100) {
    limit = 20; // TODO perf tests on large datasets
  }

  var item = new Item();
  item.update('accountId', accountId);
  item.update('lastItemRead', lastItemRead);
  item.update('limit', limit);
  item.update('offset', offset);

  item.getAuthorItems(function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      rawData: item.rows
    });
  });
});

/**!
 * ### GET /timeline/
 * Retrieve Timeline
*/
app.get('/timeline/', verifySession, function (req, res) {
  var accountId = parseInt(req.session.accountId);
  var lastItemRead = parseInt(req.query.timelineid) || 0;
  var offset = parseInt(req.query.offset) || 0;
  var direction;
  if (!direction) {
    direction = 'next';
  }
  direction = req.query.direction;

  // set max limit
  var limit = parseInt(req.query.limit);
  if (typeof limit === 'number') {
    if (limit > 30) {
      limit = 20;
    }
  } else {
    limit = 10;
  }

  var item = new Item();
  item.update('accountId', accountId);
  item.update('lastItemRead', lastItemRead);
  item.update('limit', limit);
  item.update('offset', offset);
  item.update('direction', direction);

  logger.info('direction: ', direction);

  item.getTimeline(function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }
    res.send({
      success: true,
      rawData: item.rows
    });
  });
});

/**!
 * ### GET /timeline-latest/
 * Retrieve Latest Timeline Items
*/
app.get('/timeline-latest/', verifySession, function (req, res) {
  var accountId = parseInt(req.session.accountId);

  // set max limit
  var limit = parseInt(req.query.limit);
  if (typeof limit === 'number') {
    if (!limit) {
      limit = 10;
    }
  } else {
    limit = 10;
  }

  var item = new Item();
  item.update('accountId', accountId);
  item.update('limit', limit);

  item.getLatestTimelineItems(function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }
    res.send({
      success: true,
      rawData: item.rows
    });
  });
});

/**!
 * ### GET /timeline-before/
 * Retrieve Timeline Items before 'beforeId'
*/
app.get('/timeline-before/', verifySession, function (req, res) {
  var accountId = parseInt(req.session.accountId);

  var beforeId = parseInt(req.query.beforeId);
  if (typeof beforeId !== 'number') {
    res.send({
      success: false,
      error: 'Error: Missing \'beforeId\' argument!'
    });
    return;
  }
  // set max limit
  var limit = parseInt(req.query.limit);
  if (typeof limit === 'number') {
    if (!limit) {
      limit = 10;
    }
  } else {
    limit = 10;
  }

  var item = new Item();
  item.update('accountId', accountId);
  item.update('limit', limit);
  item.update('beforeId', beforeId);

  item.getTimelineItemsBefore(function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }
    res.send({
      success: true,
      rawData: item.rows
    });
  });
});

/**!
 * ### GET /timeline-after/
 * Retrieve Timeline Items after 'afterId'
*/
app.get('/timeline-after/', verifySession, function (req, res) {
  var accountId = parseInt(req.session.accountId);

  var afterId = parseInt(req.query.afterId);
  if (typeof afterId !== 'number') {
    res.send({
      success: false,
      error: 'Error: Missing \'afterId\' argument!'
    });
    return;
  }
  // set max limit
  var limit = parseInt(req.query.limit);
  if (typeof limit === 'number') {
    if (!limit) {
      limit = 10;
    }
  } else {
    limit = 10;
  }

  var item = new Item();
  item.update('accountId', accountId);
  item.update('limit', limit);
  item.update('afterId', afterId);

  item.getTimelineItemsAfter(function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }
    res.send({
      success: true,
      rawData: item.rows
    });
  });
});
