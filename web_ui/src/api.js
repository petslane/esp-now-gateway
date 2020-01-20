import fetch from 'node-fetch';
import ApiError from "./apiError";

let url = 'http://now-gw.local/api';
if (process.env.NODE_ENV === 'production') {
    url = 'http://' + location.host + '/api';
}

export default (type, data) => {
    let params = {
        method: 'POST',
        body: JSON.stringify({
            type,
            ...data,
        }),
        headers: { 'Content-Type': 'application/json' },
        credentials: 'include',
    };

    return fetch(url, params)
        .then(res => res.json())
        .then(json => {
            if (!json.status) {
                throw new ApiError(json);
            }

            return json;
        });
};
