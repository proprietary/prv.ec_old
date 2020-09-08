import * as flatbuffers from './vendor/flatbuffers/flatbuffers';
import { ShortenerClient, LookupClient } from './client';

(function () {
	console.log('hello typescript');

	(document.querySelector('#urlForm') as HTMLFormElement).addEventListener(
		'submit',
		async (evt: Event) => {
			evt.preventDefault();
			const form = evt.target;
			const urlInputElement = document.querySelector(
				'#url',
			) as HTMLInputElement;
			const url = urlInputElement.value.trim();
			const client = new ShortenerClient();
			// TODO: display progress bar
			const shortened_url_route = await client.shorten(url);
			const shortened_url =
				window.location.protocol +
				'//' +
				window.location.host +
				'/' +
				shortened_url_route;
			const outputLink = document.querySelector(
				'#shortenedUrlResultLink',
			) as HTMLAnchorElement;
			outputLink.href = shortened_url;
			outputLink.textContent = shortened_url;
		},
	);

	if (window.location.pathname != '/') {
		(async () => {
			const identifier = window.location.pathname.slice(1);
			const pass = window.location.hash.slice(1);
			const client = new LookupClient(identifier, pass);
			const urlPlaintext = await client.lookup();
			console.info(urlPlaintext);
			window.location.replace(urlPlaintext);
		})();
	}
})();
