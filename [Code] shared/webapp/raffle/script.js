let RG;


// 监听点击事件（抽奖文本加载）
document.getElementById('fileInput').addEventListener('change', function(e) {
    const file = e.target.files[0];
    if (file) {
        const reader = new FileReader();
        reader.onload = function(event) {
            RG = JSON.parse(event.target.result);
            document.getElementById('wait').style.display = 'none';
            loadPanel();
            giftList();
        };
        reader.readAsText(file);
    }
});

// 装载抽奖面板
function loadPanel() {
    document.getElementById('getGift').style.display = 'block';
    document.title = RG["name"];
    document.querySelector('#getGift>h1').innerHTML = RG["name"];
    document.querySelector('#getGift>h2').innerHTML = RG["description"];
}

// 奖品公示
function giftList() {
    let result = " ";
    Object.entries(RG["gifts"]).forEach(([key, value]) => {
        document.querySelector("#content").innerHTML += `<li>${key}</li>`;
        result += ` ${key}(${value} 份) |`;
    });
    document.querySelector("div#probability").innerText+=result.slice(0, -1);
}


var runner = false;
var index = 0;
var interval = null;
// 抽奖函数 (监听事件)
document.getElementById('start').addEventListener('click', function() {
    let list = document.getElementById('content').children;
    let len = list.length;


    // 防止多次点击调用
    if (runner) return;
    runner = true;
    
    // 取运行的随机数
    remain = 3000 + Math.random() * 5000;
    
    interval = setInterval(function() {
        if(remain < 200){
            runner = false;
            document.getElementById('whois').innerText = list[index].innerText;
            alert('🎉 抽中了：'+list[index].innerText);
            clearInterval(interval);
        }else{
            list[index].className = '';
            list[(index+1) % len].className = 'current';
            index = ++index % len;
            document.getElementById('whois').innerText = list[index].innerText;
            remain -= 100;
        }
    },100);

});