import os
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.multioutput import MultiOutputRegressor
from sklearn.ensemble import GradientBoostingRegressor
from sklearn.metrics import mean_squared_error
from sklearn.externals import joblib

target_file = 'uv_pivot.csv'
df = pd.read_csv(os.path.join('static', 'dataset', target_file))

# 按列分割数据
df_X = df.iloc[:, :-3]
df_Y = df.iloc[:, -3:]

X_train, X_test, Y_train, Y_test = train_test_split(df_X, df_Y, test_size=0.2, random_state=42)

# 多输出 梯度下降
multi_gbr = MultiOutputRegressor(
    GradientBoostingRegressor(n_estimators=100, learning_rate=0.1, max_depth=1, random_state=0, loss='ls')
)

multi_gbr.fit(X_train, Y_train)

Y_pred = multi_gbr.predict(X_test)

rmse = mean_squared_error(Y_test, Y_pred)

print('RMSE: ', rmse)

joblib.dump(multi_gbr, os.path.join('static', 'dataset', 'multi_gbr.pkl'))
