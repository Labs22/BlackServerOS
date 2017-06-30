/* Crypton Server, Copyright 2014 SpiderOak, Inc.
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

describe('Attack Suite', function () {
  this.timeout(100000);

  describe('Attack all routes', function () {

    // Send badly formed JSON via XHR to the server for each route
    it('Attack /account route with malformed JSON', function (done) {
      superagent.post(crypton.url() + '/account')
      .withCredentials()
      .send('{"username":"foo"')
      .end(function (res) {
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'undefined is not a valid username');
        done();
      });
    }); //end it

    it('Attack /account/notSoSmart authorization route with malformed JSON', function (done) {
      superagent.post(crypton.url() + '/account/notSoSmart')
      .withCredentials()
      .send('{%$L:0909nsjdhsj]')
      .end(function (res) {
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'Invalid SRP A value.');
        done();
      });
    });

    // XXXddahl: these next two are full of fail as phantom does not like websockets:(
    // it('Attack /inbox route with malformed JSON', function (done) {
    //   superagent.get(crypton.url() + '/inbox')
    //   .withCredentials()
    //   .query('{%$L:nsdjkbsald ufahsdufnhusdhfohdfhiax djf @ ?? 0909nsjdhsj]')
    //   .end(function (res) {
    //     // Server should just ignore anything in the queryString
    //     assert.equal(res.body.success, true);
    //     assert.equal(res.body.messages, []);
    //     done();
    //   });
    // });

    // it('Attack /inbox/:messageId route with malformed JSON', function (done) {
    //   superagent.get(crypton.url() + '/inbox/1')
    //   .withCredentials()
    //   .query('{%$L:nsdjkbsald ufahsdufnhusdhfohdfhiax djf @ ?? 0909nsjdhsj]')
    //   .end(function (res) {
    //     // Server should just ignore anything in the queryString and
    //     // return no message as it does not exist or is someone else's message
    //     assert.equal(res.body.success, false);
    //     assert.equal(res.body.error, '');
    //     done();
    //   });
    // });

    it('Attack /peer/:username route with JSON', function (done) {
      superagent.get(crypton.url() + '/peer/foobar')
      .withCredentials()
      .query('{"foo":"bar"}')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        // return no message as it does not exist or is someone else's message
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'Account not found.');
        done();
      });
    });

    it('Attack /peer route with real JSON', function (done) {
      superagent.post(crypton.url() + '/peer')
      .withCredentials()
      .send('{"toAccountId": 2, "fromAccountId": 1, "headersCiphertext": "alhjkdshjfhjsdhfjkhsdjfhldshfk73487827947389646328", "payloadCiphertext": "778y23uhr2798he723hd8o72g3du2837ougo872uig3oeu23g9euog239u"}')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        // return no message as it does not exist or is someone else's message
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'Recipient account object must have accountId');
        done();
      });
    });

    it('Attack /peer route with malformed JSON', function (done) {
      superagent.post(crypton.url() + '/peer')
      .withCredentials()
      .send('{"toAccountId": 2, "fromAccountId": 1, "headersCiphertext": "alhjkdshjfhjsdhfjkhsdjfhldshfk73487827947389646328", "payloadCiphertext": "778y23uhr2798he723hd8o72g3du2837ougo872uig3oeu23g9euog239u')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        // return no message as it does not exist or is someone else's message
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'Recipient account object must have accountId');
        done();
      });
    });

    it('Attack /peer:username route with malformed JSON', function (done) {
      superagent.get(crypton.url() + '/peer/notSoSmart')
      .withCredentials()
      .query('"baz={}@778y23uhr2798he723hd8o72g3du2837ougo872uig3oeu23g9euog239u')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        assert.equal(res.body.success, true);
        assert.equal(res.body.peer.username, 'notSoSmart');
        done();
      });
    });

    it('Attack /container/:containerNameHmac route with garbage qs', function (done) {
      superagent.get(crypton.url() + '/container/1234567890&5%$%#*#7373637')
      .withCredentials()
      .query('foo="{}@778y23uhr2798he723hd8o72g3du2837ougo872uig3oeu23g9euog239u')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'No new records');
        done();
      });
    });

    it('Attack /container/:containerNameHmac:recordVersionIdentifier route with garbage qs', function (done) {
      superagent.get(crypton.url() + '/container/1234567890&5%$%#*#7/27')
      .withCredentials()
      .query('foo="{}@778y23uhr2798he723hd8o72g3du2837ougo89euog239u')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'No new records');
        done();
      });
    });

    var transactionId;

    it('Attack /transaction/create', function (done) {
      superagent.post(crypton.url() + '/transaction/create')
      .withCredentials()
      .send('foo="{}@778y23uhr2798he723hd8o72g3du2837ougo89euog239u')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        //  create a transaction
        assert.equal(res.body.success, true);
        assert(res.body.id > 1);
        transactionId = res.body.id;
        done();
      });
    });

    it('Attack /transaction/:transactionId', function (done) {
      superagent.post(crypton.url() + '/transaction/' + transactionId)
      .withCredentials()
      .send('{"addFooRecord": "wacka wacka!"}')
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        //  create a transaction
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'Invalid transaction type');
        done();
      });
    });

    it('Attack /transaction/:transactionId/commit', function (done) {
      superagent.post(crypton.url() + '/transaction/' + transactionId + '/commit')
      .withCredentials()
      .end(function (res) {
        // Server should just ignore anything in the queryString and
        // commit a transaction
        // XXXddahl: this appears to commit an empty transaction
        assert.equal(res.body.success, true);
        done();
      });
    });

    it('Attack /transaction/:transactionId', function (done) {
      superagent.del(crypton.url() + '/transaction/' + transactionId)
      .withCredentials()
      .send('{[ahsjdhashdhs3767^&^&}')
      .end(function (res) {
        // Server should just ignore anything sent and
        // delete the transaction
        assert.equal(res.body.success, true);
        done();
      });
    });

    it('Attack /transaction/:transactionId - attempt to delete transaction not owned', function (done) {
      superagent.del(crypton.url() + '/transaction/1234')
      .withCredentials()
      .send('{[ahsjdhashdhs3767^&^&}')
      .end(function (res) {
        // Server should just ignore anything sent and
        // delete the transaction
        assert.equal(res.body.success, false);
        assert.equal(res.body.error, 'Transaction does not belong to account');
        done();
      });
    });

  });
});
