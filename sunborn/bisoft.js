var prefix = 'pages/';


function loadintoIframe(iframeid, url)
{
	if (document.getElementById)
		document.getElementById(iframeid).src=url;
}

function GetPageFromUrl()
{
	var page = 'home';

	var hasQueryString = document.URL.indexOf('?');
	if (hasQueryString != -1) {
		// Create variable from ? in the url to the end of the string
		page = document.URL.substring(hasQueryString+1, document.URL.length);
	}
	page = prefix + page + '.htm';
	loadintoIframe('ibody', page);
}