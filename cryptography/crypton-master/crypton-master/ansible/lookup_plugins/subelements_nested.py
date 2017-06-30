import ansible.utils as utils
import ansible.errors as errors


class noflatten(tuple):
    pass


def flatten(terms):
    ret = []
    for term in terms:
        if isinstance(term, list):
            ret.extend(term)
        elif isinstance(term, tuple):
            ret.extend(term)
        else:
            ret.append(term)
    return ret

def combine(a,b):
    results = []
    for x in a:
        for y in b:
            results.append(flatten([x,y]))
    return results

def do_nest(term0, terms):
    my_list = [[term0]] + terms[:]
    my_list.reverse()
    result = []
    result = my_list.pop()
    while len(my_list) > 0:
        result2 = combine(result, my_list.pop())
        result  = result2
    new_result = []
    for x in result:
        new_result.append(flatten(x))
    return new_result


class LookupModule(object):

    def __init__(self, basedir=None, **kwargs):
        self.basedir = basedir

    def __lookup_injects(self, terms, inject):
        results = []
        for x in terms:
            intermediate = utils.listify_lookup_plugin_terms(x, self.basedir, inject)
            results.append(intermediate)
        return results

    def run(self, terms, inject=None, **kwargs):
        terms = utils.listify_lookup_plugin_terms(terms, self.basedir, inject)
        terms[0] = utils.listify_lookup_plugin_terms(terms[0], self.basedir, inject)
        terms[2:] = self.__lookup_injects(terms[2:], inject)

        if not isinstance(terms, list) or len(terms) < 3:
            raise errors.AnsibleError(
                "subelements_nested lookup expects a list of at least three items, first a dict or a list, second a string, and one or more lists")
        terms[0] = utils.listify_lookup_plugin_terms(terms[0], self.basedir, inject)
        if not isinstance(terms[0], (list, dict)) or not isinstance(terms[1], basestring):
            raise errors.AnsibleError(
                "subelements_nested lookup expects a list of at least three items, first a dict or a list, second a string, and one or more lists")

        if isinstance(terms[0], dict): # convert to list:
            if terms[0].get('skipped',False) != False:
                # the registered result was completely skipped
                return []
            elementlist = []
            for key in terms[0].iterkeys():
                elementlist.append(terms[0][key])
        else: 
            elementlist = terms[0]
        subelement = terms[1]

        nest_terms = terms[2:]

        ret = []
        for item0 in elementlist:
            if not isinstance(item0, dict):
                raise errors.AnsibleError("subelements lookup expects a dictionary, got '%s'" %item0)
            if item0.get('skipped',False) != False:
                # this particular item is to be skipped
                continue 
            if not subelement in item0:
                raise errors.AnsibleError("could not find '%s' key in iterated item '%s'" % (subelement, item0))
            if not isinstance(item0[subelement], list):
                raise errors.AnsibleError("the key %s should point to a list, got '%s'" % (subelement, item0[subelement]))
            sublist = item0.pop(subelement, [])
            for item1 in sublist:
                ret.extend(do_nest((item0, item1), nest_terms))

        return ret
