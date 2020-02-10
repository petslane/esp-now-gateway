
class ApiError extends Error {
    constructor(data) {
        super(data.error);

        this.json = data;
    }
}

export default ApiError;