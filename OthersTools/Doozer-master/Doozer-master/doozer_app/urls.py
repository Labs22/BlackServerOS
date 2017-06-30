from django.conf.urls import patterns, include, url
from django.contrib import admin
admin.autodiscover()


urlpatterns = patterns('',
    url(r'^$', 'doozer_app.views.home', name='home'),
    url(r'^submit/', 'doozer_app.views.submit'),
    url(r'^check/', 'doozer_app.views.check'),
    url(r'^fetch/', 'doozer_app.views.fetch'),
    url(r'^pwlist/', 'doozer_app.views.master_password_list'),
    url(r'^hashlist/', 'doozer_app.views.master_hash_list'),
    url(r'^sessions/', 'doozer_app.views.sessions'),
    url(r'^session/(.*?)/$', 'doozer_app.views.session_fetch'),
    url(r'^admin/', include(admin.site.urls)),
)
