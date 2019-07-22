

# include "jacobi.hpp"
void solveEquation(vector<map<int,double> > & matrixA,map<int,double> & bMatrix,vector<double> & result){
    int numberOfVar=(int)bMatrix.size();
    result.clear();
    
    for (int i=0;i<numberOfVar;i++){
        result.push_back(0);
    }
    bool continu=true;
    int k=0;
    while (continu && k<MAX_ITER_JACOBI){
        vector<double> prevResult(result);
        for (int i=0;i<numberOfVar;i++){
            double weightedSum=0;
            double a_ii=0;
            for (map<int,double>::iterator it=matrixA[i].begin();it!=matrixA[i].end();it++){
                if (it->first!=i){
                    weightedSum+=it->second*result[it->first];
                }
                else {
                    a_ii=it->second;
                }
                
            }
            result[i]=(bMatrix[i]-weightedSum)/a_ii;
        }
        continu=false;
        k++;
        for (int i=0;i<numberOfVar;i++){
            double error=result[i]-prevResult[i];
            if (error<0){
                error*=-1;
            }
            if (error>MIN_APPROX_JACOBI){
                continu=true;
                break;
            }
        }
    }
}
