from django.template import RequestContext
from django.shortcuts import render_to_response
from django.db.models import Q
from django.http import HttpResponse, HttpResponseRedirect
from django.views.decorators.csrf import csrf_exempt
from forms import SearchForm
from models import HashModel
from os import listdir, path
import doozer.core.sethor as sethor


def home(request):
    """ Manages our home page.  If a POST request is received, then
    we're going to pull the search value out and attempt to filter
    for it.  This can be a hash or a plaintext value.
    """
    
    if request.method == 'POST':
        form = SearchForm(request.POST)

        if form.is_valid():
            data =  HashModel.objects.all()
            search = request.POST.get("search_input")

            # search the hashval and plaintext fields 
            result = data.filter(Q(hashval=search) | Q(plaintext=search))

            return render_to_response('home.html',
                            {'form':SearchForm(),
                             'results':result
                            },
                            context_instance=RequestContext(request))

    return render_to_response("home.html", 
                                {'form' : SearchForm(),
                                 'results':HashModel.objects.all()[:50]
                                },
                                context_instance=RequestContext(request))


@csrf_exempt
def submit(request):
    """ This view manages the /submit/ function, which adds an entry to the
    database.

    POST data should be: hash=[HASH]&val=[PLAINTEXT]&hashtype=[HASH TYPE]

    This can trivially be hit with the following requests code:

        requests.post("http://url/submit", 
                        data={"hash":"value", "val":"plaintext", "hashtype":"hashtype"})
    """

    if request.method == 'POST':

        hashval = request.POST.get('hash')
        plaintext = request.POST.get('val')
        hash_type = request.POST.get('hashtype')

        if HashModel.objects.filter(hashval=hashval, plaintext=plaintext).exists():
            # update hit count
            obj = HashModel.objects.get(hashval=hashval, plaintext=plaintext)
            obj.hit_count += 1
            obj.save()
        else:
            # create new entry
            hm = HashModel(hashval = hashval, plaintext = plaintext, hash_type=hash_type)
            hm.save()

    return render_to_response("home.html", {'form' : SearchForm()},
                        context_instance=RequestContext(request))


@csrf_exempt
def check(request):
    """ Manages the /check/ function, which simply returns True or False 
    for whether or not a hash exists in the database.

    POST data should be hash=[HASH] OR plaintext=[PLAINTEXT]
    """

    if request.method == 'POST':

        val = None
        if 'hash' in request.POST:
            val = request.POST.get("hash")
        elif 'val' in request.POST:
            val = request.POST.get('val')

        return HttpResponse(str(HashModel.objects.filter(Q(hashval=val) | \
                                           Q(plaintext=val)).exists()))
        
    return render_to_response("home.html", {'form' : SearchForm()},
                         context_instance=RequestContext(request))


@csrf_exempt
def fetch(request):
    """ Manages the /fetch/ function, which looks up a given hash in the database
    and returns its respective plaintext counterpart.

    POST data should be hash=[HASH]&hashtype=[HASH TYPE]
    """

    if request.method == 'POST':
        hashval = str(request.POST.get('hash'))
        hashtype = str(request.POST.get('hashtype'))
        
        try:
            # ignore hashtype for now; weird bug with it
            data = HashModel.objects.get(hashval=hashval)
            return HttpResponse(data.plaintext)
        except Exception, e:
            return HttpResponse("%s:%s - %s" % (hashval, hashtype, e))

    return HttpResponseRedirect("/")


@csrf_exempt
def master_password_list(request):
    """ Dumps the master password list to the request
    """

    passwords = HashModel.objects.values_list('plaintext', flat=True)
    return HttpResponse('\n'.join(passwords), content_type='text/plain')


@csrf_exempt
def master_hash_list(request):
    """ Dumps the master hash list to the request.  A simple GET will return all
    hashes in the database, while a POST specifying hashtype will return all hashes
    of the requested type.
    """

    if request.method == "POST":
        hashtype = request.POST.get("hashtype")
        hashes = HashModel.objects.values_list('hashval', flat=True).filter(hash_type=hashtype)
    else:
        hashes = HashModel.objects.values_list('hashval', flat=True)
    
    return HttpResponse('\n'.join(hashes), content_type='text/plain')


def sessions(request):
    """ Simple dump of all sessions and their state from the sessions directory
    """

    # [0] = session name
    # [1] = last line in log
    # [2] = Running | Completed            
    sessions = []

    if not path.exists(sethor.WORKING_DIR):
        return render_to_response("sessions.html", {"sessions":sessions},
                                context_instance=RequestContext(request))

    # poll doozer/sethor.py for the sessions directory
    session_list = listdir(sethor.WORKING_DIR)

    # build our session object
    for session in session_list:

        tmp = []
        tmp.append(session)

        try:
            lines = open("%s/%s/hor.log" % (sethor.WORKING_DIR, session)).readlines()
        except:
            # if there's no hor.log, that means we're queued
            sessions.append([session, "waiting...", "Queued"])
            continue

        tmp.append(lines[len(lines)-1])

        if 'successfully completed' in lines[len(lines)-1]:
            tmp.append("Completed")
        else:
            tmp.append("Running")

        sessions.append(tmp)

    return render_to_response("sessions.html", {"sessions":sessions},
                                context_instance=RequestContext(request))

def session_fetch(request, gsession):
    """ If a job has completed, this will enable a link on the state column for
    that particular job, allowing you the ability to view the results of that
    session.
    """
    
    if len(gsession) <= 1:
        return HttpResponse("")

    if not path.exists(sethor.WORKING_DIR):
        return HttpResponse("")

    # check if this session exists and, if so, return the pot data
    session_list = listdir(sethor.WORKING_DIR)
    for session in session_list:

        files = listdir("%s/%s" % (sethor.WORKING_DIR, session))
        if not gsession in session:
            continue
        
        for f in files:
            if '.pot' in f:
                data = open("{0}/{1}/{2}".format(sethor.WORKING_DIR, session, f)).readlines()
                return HttpResponse(data, content_type="text/plain")

    return HttpResponse("")
