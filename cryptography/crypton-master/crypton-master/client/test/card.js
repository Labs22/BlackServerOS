/* Crypton Client, Copyright 2013, 2014, 2015 SpiderOak, Inc.
 *
 * This file is part of Crypton Client.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

'use strict';

var assert = chai.assert;

describe('Card', function() {
  var card = new crypton.Card();
  var username = 'drzhivago';
  var appname = 'noneofyourfingbizness';
  var fingerprint = 'fbc42d5f1dc4e42b3b02338eef5364670461f4bb02b80b66263407011619d092';
  var url = 'https://spideroak.com/solutions/crypton-framework';

  // helper functions //////////////////////////////////////////////////////

  function getRGBFromHex(hex) {
    var bigint = parseInt(hex, 16);
    var r = (bigint >> 16) & 255;
    var g = (bigint >> 8) & 255;
    var b = bigint & 255;
    return [r, g, b];
  }

  function testCanvasColorAt(hexcolor, canvas, x, y) {
    var ctx = canvas.getContext('2d');
    var imageData = ctx.getImageData(x, y, 1, 1); // get one pixel
    var data = imageData.data;

    var hex = hexcolor.substring(1, 7);
    var expectedRGB = getRGBFromHex(hex);
    var red;
    var green;
    var blue;

    red = data[0];
    green = data[1];
    blue = data[2];
    assert.equal(red, expectedRGB[0]);
    assert.equal(green, expectedRGB[1]);
    assert.equal(blue, expectedRGB[2]);
  }

  // end helper functions /////////////////////////////////////////////////

  describe('createFingerprintArr()', function() {
    it('should create an array with 16 members', function(done) {
      var arr = card.createFingerprintArr(fingerprint);
      assert.equal(arr.length, 16);
      done();
    });
  });

  describe('createColorArr()', function() {
    it('should create an array of hex color values', function(done) {
      var fingerArr = card.createFingerprintArr(fingerprint);
      var arr = card.createColorArr(fingerArr);
      assert.equal(arr.length, 16);
      assert.equal(arr[0].length, 7);
      assert.equal(arr[0][0], '#');
      done();
    });
  });

  describe('createQRCode()', function() {
    it('should generate a QR code in a canvas element', function(done) {
      var fingerArr = card.createFingerprintArr(fingerprint);
      var canvas = card.createQRCode(fingerArr, username, appname, url);
      testCanvasColorAt('#000000', canvas, 100, 100);
      done();
    });
  });

  describe('createIdentigrid()', function() {
    it('should generate a grid of colors based on the fingerprint', function(done) {
      var fingerArr = card.createFingerprintArr(fingerprint);
      var colorArr = card.createColorArr(fingerArr);

      var canvas = card.createIdentigrid(colorArr);
      testCanvasColorAt(colorArr[0], canvas, 2, 2);
      testCanvasColorAt(colorArr[1], canvas, 52, 2);
      done();
    });
  });

  describe('createIDCard()', function() {
    it('should generate the full ID Card', function(done) {
      var fingerArr = card.createFingerprintArr(fingerprint);
      var colorArr = card.createColorArr(fingerArr);

      var domId = 'my-dom-id-is-the-best';
      var idCard = card.createIdCard(fingerprint, username, appname, url, domId);
      assert.equal(idCard.tagName, 'CANVAS');

      // testCanvasColorAt(255, idCard, 12, 205); // need to re-think these tests as the card design changed a little
      done();
    });
  });

  ////////////////////////////////////////////////////////////////////////
  // XXXddahl TODO: More tests on additional x,y canvas colors.
  //                Parse QR code canvas into data
  ////////////////////////////////////////////////////////////////////////
});
