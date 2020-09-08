import { ShortenerClient, LookupClient } from './client';

(function () {
	(document.querySelector('#urlForm') as HTMLFormElement).addEventListener(
		'submit',
		async (evt: Event) => {
			evt.preventDefault();
			const form = evt.target;
			const urlInputElement = document.querySelector(
				'#url',
			) as HTMLInputElement;
			let url = urlInputElement.value.trim();
			// when urls are entered into the box without a protocol, assume it's https
			if (!/^(https?:\/\/)|(mailto:)/.test(url)) {
				url = "https://" + url;
			}
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

	if (window.location.pathname != '/' && window.location.pathname != '/index.html') {
		(async () => {
			try {
				const identifier = window.location.pathname.slice(1);
				const pass = window.location.hash.slice(1);
				if (identifier.length === 0  || pass.length === 0) {
					return;
				}
				const client = new LookupClient(identifier, pass);
				// TODO: display progress bar while deriving key
				const urlPlaintext = await client.lookup();
				console.info(`redirecting to: ${urlPlaintext}`);
				window.location.replace(urlPlaintext);
			} catch (e) {
				console.error(e);
				window.location.replace('/');
			}
		})();
	}
})();
