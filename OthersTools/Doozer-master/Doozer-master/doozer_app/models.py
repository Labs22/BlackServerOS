from django.db import models
from django.contrib import admin
from config import SUPPORTED_HASHES


class HashModel(models.Model):
    """ Main hash model
    """

    hashval   = models.CharField(max_length=50, unique=True)
    plaintext = models.CharField(max_length=100)
    hash_type = models.CharField(max_length=100, choices=SUPPORTED_HASHES, default="nt")
    hit_count = models.IntegerField(default=1)

    def __unicode__(self):
        return "%s:%s"  % (self.hashval, self.plaintext)


admin.site.register(HashModel)
