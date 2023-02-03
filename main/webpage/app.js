let timer

function startUpdates() {
    fetch('/btnStartUpdates')
    .then(response => response.json())
    .then(json => {
        console.log(json.update)
    })
    .catch(err => console.log('Request failed', err));

    // clear timer if already used
    if (timer){
        clearInterval(timer)
    }
    timer = setInterval(update, 1000)
}

function update() {
    let lblUpdate = document.getElementById("lblUpdate")
    fetch('/getUpdate')
    .then(response => response.json())
    .then(json => {
        lblUpdate.innerHTML = json.update
        console.log(json.update)
    })
    .catch(err => console.log('Request failed', err));
}

function stopUpdates() {
    
    if (timer){
        clearInterval(timer)
    }
    x = 0
    let lblUpdate = document.getElementById("lblUpdate")
    lblUpdate.innerHTML = "no Status available"

    fetch('/btnStopUpdates')
    .then(response => response.json())
    .then(json => {
        console.log(json.update)
    })
    .catch(err => console.log('Request failed', err));
}

function setOperator () {
    let inputOperator = document.getElementById("inputOperator")

    if (inputOperator.value == '')
        window.alert("Please type the name of the operator first")
    else
    {
        data = inputOperator.value

        fetch("/btnSetOperator",
        {
            method: "POST",
            body: data,
            headers: {"Content-type": "text/plain; charset=UTF-8"}
        })
        .then(res => res.json())
        .then(json => console.log(json.reply))
        .catch(err => console.log(err));

        inputOperator.value = '';
    }
}

function btnStart () {
    fetch('/btnStart')
    .then(response => response.json())
    .then(json => {
        console.log(json.reply)
    })
    .catch(err => console.log('Request failed', err));
}


function btnStop () {
    fetch('/btnStop')
    .then(response => response.json())
    .then(json => {
        console.log(json.reply)
    })
    .catch(err => console.log('Request failed', err));
}
