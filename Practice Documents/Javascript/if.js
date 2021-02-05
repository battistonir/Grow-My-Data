let number=[];
numbers=[1,2,1,2,3,2,3,1];
if(number[0]==number[2]){
    console.log('Correct');
}

if(numbers[0]==numbers[4]){
    console.log('Correct');
}
else{
    console.log('wrong,try again');
}

if(numbers[0]==numbers[2]&&numbers[2]==numbers[6]){
    console.log("Correct");
}
else{
    console.log('Wrong,try again');
}

if(numbers[0]==numbers[2]||numbers[2]==numbers[6]){
    console.log("Correct");
}
else{
    console.log('Wrong,try again');
}