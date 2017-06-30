# -*- coding: utf-8 -*-
"""
Python web spider/crawler based on scrapy with support for POST/GET login,
variable level of recursions/depth and optionally save to disk.

This file provides the a middleware that overwrites the crawling depth behavior.
"""

from scrapy.spidermiddlewares.depth import DepthMiddleware


__author__ = "cytopia"
__license__ = "MIT"
__email__ = "cytopia@everythingcli.org"


"""Custom DepthMiddleWare"""
class MyDepthMiddleware(DepthMiddleware):

    #----------------------------------------------------------------------
    def process_spider_output(self, response, result, spider):
        """Overwrite parent DepthMiddleware and set MAX_DEPTH"""

        if hasattr(spider, 'max_depth'):
            self.maxdepth = getattr(spider, 'max_depth')
        return super(MyDepthMiddleware, self).process_spider_output(response, result, spider)
