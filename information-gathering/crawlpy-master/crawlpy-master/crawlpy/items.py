# -*- coding: utf-8 -*-
"""
Python web spider/crawler based on scrapy with support for POST/GET login,
variable level of recursions/depth and optionally save to disk.

Defines my custom model items.
See documentation in:
http://doc.scrapy.org/en/latest/topics/items.html
"""

from scrapy.item import Item, Field


__author__ = "cytopia"
__license__ = "MIT"
__email__ = "cytopia@everythingcli.org"


class CrawlpyItem(Item):
    """Data Model Class"""

    url = Field()
    text = Field()
    status = Field()
    depth = Field()
    referer = Field()
