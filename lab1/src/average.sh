sum=0
numbers=0
for a in {1..150}; do
  read b
  for j in $b; do
    [[ $j =~ ^[0-9]+$ ]] || { echo "$j is not a number" >&2 && exit 1; } 
    ((numbers+=1)) && ((sum+=j))
  done
done

((numbers==0)) && avg=0 || avg=$(echo "$sum / $numbers" | bc -l)
echo "Sum of inputs = $sum"
echo "Number of inputs = $numbers"
printf "Average input = %.2f\n" $avg 