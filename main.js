const addon = require("./build/Release/addon");

const print = console.log.bind(console);

const doAsyncStuff = promisify(addon.DoAsyncStuff)

const dataPromise = new Promise(resolve => {
    setTimeout(() => resolve({ data: "bleep bloop" }), 5000);
})

doAsyncStuff(dataPromise)
    .then(result => console.log(`success: "${result}"`))
    .catch(err => console.log(`failure:, "${err}"`))

function promisify(functionTakesCallback) {
    return function() {
        return new Promise((resolve, reject) => {
            functionTakesCallback(...arguments, function(err, res) {
                if (err != null) reject(err)
                else resolve(res)
            })
        })
    }
}
