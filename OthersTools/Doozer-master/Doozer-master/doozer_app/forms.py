from django import forms

class SearchForm(forms.Form):
    """ Adds a search bar to our home page
    """

    search_input = forms.CharField(label='', required=True, max_length=200)
