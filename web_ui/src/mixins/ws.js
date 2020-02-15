export default {
    methods: {
        sendWebSocketMessage(type, to, message, id) {
            /**
             * @param {WebSocket}
             */
            const ws = this.$ws;

            ws.send(JSON.stringify({
                type,
                to,
                id: (id || new Date()*1) % 4294967295,
                message: message.substring(0, 250),
            }));
        }
    }
};
