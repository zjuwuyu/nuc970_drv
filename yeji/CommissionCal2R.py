class Calculator2R:
    def __init__(self, base, gp):
        self.base = base
        self.gp = gp
        self.cost = Calculator2R.cal_cost(self.base)
        self.commission = Calculator2R.cal_commission(self.gp, self.cost)
        self.annual_package = Calculator2R.cal_package(self.base, self.commission)

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
        cost = 2.6 * base * 12 + 2 * base * 4 * 2
        return cost

    @staticmethod
    def cal_commission(gp, cost):
        commission = 0
        if gp < 90:
            commission = 0
        elif gp < 110:
            commission = (gp * 0.94 - cost) * 0.25
        elif gp < 140:
            commission = (gp * 0.94 - cost) * 0.3
        elif gp < 170:
            commission = (gp * 0.94 - cost) * 0.35
        else:
            commission = (gp * 0.94 - cost) * 0.4
        return commission

    @staticmethod
    def cal_package(base, commission):
        package = base*12 + commission
        return package
