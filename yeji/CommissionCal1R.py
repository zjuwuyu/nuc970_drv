class Calculator1R:
    def __init__(self, base, gp):
        self.base = base
        self.gp = gp
        self.cost = Calculator1R.cal_cost(base)
        self.commission = Calculator1R.cal_commission(self.gp, self.cost)
        self.annual_package = Calculator1R.cal_package(self.base, self.commission)

    def populate_data(self):
        data = {}
        data["base"] = int(self.base)
        data["gp"] = int(self.gp)
        data["cost"] = int(self.cost)
        data["commission"] = int(self.commission)
        data["annual_package"] = int(self.annual_package)
        return data

    @staticmethod
    def cal_cost(base):
        cost = 2.6 * base * 12 + 2 * base * 4
        return cost

    @staticmethod
    def cal_commission(gp, cost):
        commission = 0
        if gp < 80:
            commission = 0
        elif gp < 100:
            commission = (gp * 0.94 - cost) * 0.25
        elif gp < 125:
            commission = (gp * 0.94 - cost) * 0.3
        elif gp < 150:
            commission = (gp * 0.94 - cost) * 0.35
        else:
            commission = (gp * 0.94 - cost) * 0.4
        return commission

    @staticmethod
    def cal_package(base, commission):
        package = base*12 + commission
        return package
