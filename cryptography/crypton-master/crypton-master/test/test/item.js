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

describe('Item tests', function () {
  this.timeout(200000);

  var alice;
  var aliceSession;
  var itemNameHmac;
  var item1;

  describe('Create Account', function () {
    it('Create Alice', function (done) {
      crypton.generateAccount('alice3', 'pass', function (err, acct) {
        if (err) throw err;
        assert(acct);
        alice = acct;
        done();
      });
    });

    it('Get Alice\'s session', function (done) {
      crypton.authorize('alice3', 'pass', function (err, sess) {
        if (err) throw err;
        aliceSession = sess;
        assert(sess);
        done();
      });
    });

    it('Create an Item', function (done) {
      aliceSession.getOrCreateItem('my-first-item', function (err, item) {
        if (err) {
          console.error(err);
          throw (err);
        }
        item1 = item;
        assert(item);
        assert(item.sessionKey);
        assert(item.value);
        itemNameHmac = item.nameHmac;
        done();
      });
    });


    it('Update Item', function (done) {
      try {
        item1.value = { foo: 1, bar: 2, baz: 3 };
      } catch (ex) {
        console.error(ex);
        throw new Error(ex);
      }
      done();
    });

    it('Verify Updated Item', function (done) {
      assert.equal(item1.value.foo, 1);
      assert.equal(item1.value.bar, 2);
      assert.equal(item1.value.baz, 3);
      done();
    });

    // remove item
    it('removes item', function (done) {
      var nameHmac = aliceSession.items['my-first-item'].nameHmac
      aliceSession.removeItem(nameHmac, function (err) {
        if (err) {
          throw new Error(err);
        }
        assert.equal(Object.keys(aliceSession.items).length, 0);
        done();
      });
    });

  });
});
