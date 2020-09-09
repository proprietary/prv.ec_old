import { ShortenerClient, LookupClient } from './client';

function showLoading() {
	const el = document.querySelector('#progressSection') as HTMLElement;
	el.style.visibility = 'visible';
	const timer = window.setInterval(() => {
		let loadingBar = document.querySelector('progress#loadingBar') as HTMLProgressElement;
		const val = +loadingBar.value;
		if (val == 100) {
			loadingBar.value = 0;
		} else {
			loadingBar.value = val + 1;
		}
	}, 100);
	return timer;
}

function hideLoading(timer: number) {
	window.clearInterval(timer);
	const el = document.querySelector('#progressSection') as HTMLElement;
	el.style.visibility = 'hidden';
}

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
			const timer = showLoading();
			const client = new ShortenerClient();
			const shortened_url_route = await client.shorten(url);
			hideLoading(timer);
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
				const timer = showLoading();
				const client = new LookupClient(identifier, pass);
				const urlPlaintext = await client.lookup();
				hideLoading(timer);
				console.info(`redirecting to: ${urlPlaintext}`);
				window.location.replace(urlPlaintext);
			} catch (e) {
				console.error(e);
				window.location.replace('/');
			}
		})();
	}
})();
